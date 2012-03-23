#include "BatsModule.h"
#include "BuildPlanner.h"
#include "Commander.h"
#include "Helper.h"
#include "SquadManager.h"
#include "UnitManager.h"
#include "GameTime.h"
#include "BTHAIModule/Source/FileReaderUtils.h"
#include "BTHAIModule/Source/AgentManager.h"
#include "BTHAIModule/Source/ExplorationManager.h"
#include "BTHAIModule/Source/CoverMap.h"
#include "BTHAIModule/Source/PathFinder.h"
#include "BTHAIModule/Source/UpgradesPlanner.h"
#include "BTHAIModule/Source/ResourceManager.h"
#include "BTHAIModule/Source/Profiler.h"
#include "Utilities/Helper.h"
#include "Utilities/Logger.h"
#include "Utilities/String.h"
#include "Config.h"
#include <cassert>

using namespace bats;
using namespace BWAPI;

const int WORKER_MINERAL_PRICE = 50;
const int GAME_STARTED_FRAME = 1;

BatsModule::BatsModule() : BTHAIModule() {
	mpProfiler = NULL;
	mpUnitManager = NULL;
	mpCommander = NULL;

	// Initialize logger
	utilities::setOutputDirectory(config::log::OUTPUT_DIR);
	utilities::loadLogSettings(config::log::SETTINGS_FILE);
	config::loadConfig();

	// Initialize singletons that stays throughout multiple games
	mpProfiler = Profiler::getInstance();
}

BatsModule::~BatsModule() {
	SAFE_DELETE(mpProfiler);

	// Release game classes, if the game was ended abruptly onEnd() might not have been called.
	releaseGameClasses();

	utilities::checkForErrors();
}

void BatsModule::onStart() {
	mpProfiler->start("OnInit");

	Broodwar->enableFlag(BWAPI::Flag::UserInput);
	// Set default speed
	Broodwar->setLocalSpeed(config::game::SPEED);

	BWTA::readMap();
	BWTA::analyze();

	profile = false;

	initGameClasses();

	if (Broodwar->isReplay()) 
	{
		Broodwar->printf("The following players are in this replay:");
		std::set<Player*>::iterator playerIt;
		for(playerIt = Broodwar->getPlayers().begin();
			playerIt != Broodwar->getPlayers().end();
			++playerIt)
		{
			if (!(*playerIt)->getUnits().empty() && !(*playerIt)->isNeutral())
			{
				Broodwar->printf("%s, playing as a %s",(*playerIt)->getName().c_str(),(*playerIt)->getRace().getName().c_str());
			}
		}
	}
	//Add the units we have from start to agent manager
	else {
		std::set<Unit*>::const_iterator unitIt;
		for(unitIt = Broodwar->self()->getUnits().begin();
			unitIt != Broodwar->self()->getUnits().end();
			++unitIt)
		{
			mpUnitManager->addAgent(*unitIt);
		}
	}

	//Broodwar->printf("BTHAI %s (%s)", VERSION.c_str(), Broodwar->self()->getRace().getName().c_str());

	running = true;

	mpProfiler->end("OnInit");
}

void BatsModule::onEnd() {
	Pathfinder::getInstance()->stop();
	mpProfiler->dumpToFile();

	releaseGameClasses();
}

void BatsModule::onFrame() {
	mpProfiler->start("OnFrame");

	// Some states to skip further processing
	if (!running || // Game over
		!Broodwar->isInGame() ||
		Broodwar->isReplay()) {

		mpProfiler->end("OnFrame");
		return;
	}

	updateGame();
	showDebug();

	mpProfiler->end("OnFrame");

	// Show profiler information if profiling is on
	if (profile) {
		mpProfiler->showAll();
	}
}

void BatsModule::onSendText(std::string text) {
	utilities::string::toLower(text);

	// /d# needs to be overridden because base class calls a class we don't initialize
	if (text == "/d1" || text == "/debug low") {
		mDebugLevel = 1;
	} else if (text == "/d2" || text == "/debug medium") {
		mDebugLevel = 2;
	} else if (text == "/d3" || text == "/debug high") {
		mDebugLevel = 3;
	} else if (text == "/d0" || text == "/debug off") {
		mDebugLevel = 4;
	} else if (text == "/transition") {
		BuildPlanner::getInstance()->switchToPhase("");
	} else if (startsWith(text,"/transition")) {				
		BuildPlanner::getInstance()->switchToPhase(text.substr(12, text.length()-12));
	} else if (mpCommander->isCommandAvailable(text)) {
		mpCommander->issueCommand(text);
	} else {
		// Default behavior
		BTHAIModule::onSendText(text);
	}
}

bool BatsModule::startsWith(const std::string& text,const std::string& token){
	
	if(text.length() < token.length() || text.length() == 0)
		return false;

	for(unsigned int i=0; i<token.length(); ++i)
	{
		if(text[i] != token[i])
			return false;
	}

	return true;
}


void BatsModule::onPlayerLeft(BWAPI::Player* player) {
	// Stop game if we left the game
	if (player->getID() == Broodwar->self()->getID()) {
		onEnd();
	}
	// Print out the player that left the game
	else {
		Broodwar->sendText("%s left the game.",player->getName().c_str());
	}
}

void BatsModule::onUnitCreate(BWAPI::Unit* pUnit) {
	if (areWePlaying()) {
		mpUnitManager->addAgent(pUnit);

		// Remove from build order if it's a building
		if (pUnit->getType().isBuilding()) {
			BuildPlanner::getInstance()->unlock(pUnit->getType());
		}
	}
}

void BatsModule::onUnitDestroy(BWAPI::Unit* pUnit) {
	if (areWePlaying()) {

		if (OUR(pUnit)) {
			mpUnitManager->removeAgent(pUnit);

			if (pUnit->getType().isBuilding()) {
				BuildPlanner::getInstance()->buildingDestroyed(pUnit);
			}

			// Assist workers under attack
			if (pUnit->getType().isWorker()) {
				/// @todo assist worker.
				/// Commander::getInstance()->assistWorker(mpAgentManager->getAgent(pUnit->getID()));
			}

			mpUnitManager->cleanup();
		}
		// Enemies
		else if (!pUnit->getType().isNeutral()) {
			ExplorationManager::getInstance()->unitDestroyed(pUnit);
		}
	}
}

void BatsModule::onMorphUnit(BWAPI::Unit* pUnit) {
	if (areWePlaying()) {
		if (OUR(pUnit)) {
			if (BuildPlanner::isZerg()) {
				AgentManager::getInstance()->morphDrone(pUnit);
				BuildPlanner::getInstance()->unlock(pUnit->getType());
			} else {
				onUnitCreate(pUnit);
			}
		}
	}
}

bool BatsModule::isGameLost() const {
	// Check if we have at least one attacking unit
	const std::vector<BaseAgent*>& agents = mpUnitManager->getAgents();
	std::vector<BaseAgent*>::const_iterator agentIt = agents.begin();
	bool attackingUnitExist = false;
	while(!attackingUnitExist && agentIt != agents.end()) {
		if ((*agentIt)->isAttacking()) {
			attackingUnitExist = true;
		}

		++agentIt;
	}

	if (mpUnitManager->getNoWorkers() == 0 &&
		Broodwar->self()->minerals() <= WORKER_MINERAL_PRICE &&
		!attackingUnitExist)
	{
		return true;
	} else {
		return false;
	}
}

bool BatsModule::areWePlaying() const {
	return !Broodwar->isReplay() && Broodwar->getFrameCount() > GAME_STARTED_FRAME;
}

void BatsModule::updateGame() {
	// Quit the game if the game is lost
	if (isGameLost()) {
		Broodwar->printf("No workers left; bailing out.");
		Broodwar->sendText("gg");
		Broodwar->leaveGame();
		return;
	}

	mpUnitManager->computeActions();
	BuildPlanner::getInstance()->computeActions();
	mpCommander->computeActions();
	ExplorationManager::getInstance()->computeActions();
}

void BatsModule::initGameClasses() {
	GameTime::getInstance();
	SquadManager::getInstance();
	mpUnitManager = UnitManager::getInstance();
	mpCommander = Commander::getInstance();
	CoverMap::getInstance();
	BuildPlanner::getInstance();
	UpgradesPlanner::getInstance();
	ResourceManager::getInstance();
	Pathfinder::getInstance();
}

void BatsModule::releaseGameClasses() {
	
	delete Pathfinder::getInstance();
	delete ResourceManager::getInstance();
	delete UpgradesPlanner::getInstance();
	delete BuildPlanner::getInstance();
	delete CoverMap::getInstance();
	SAFE_DELETE(mpCommander);
	SAFE_DELETE(mpUnitManager);
	delete SquadManager::getInstance();
	delete GameTime::getInstance();
}

void BatsModule::showDebug() const {
	BuildPlanner::getInstance()->printInfo();
	if (mDebugLevel > 0) {
		std::vector<BaseAgent*> agents = mpUnitManager->getAgents();
		for (int i = 0; i < (int)agents.size(); i++) {
			if (agents.at(i)->isBuilding()) agents.at(i)->debug_showGoal();
			if (!agents.at(i)->isBuilding() && mDebugLevel >= 2) agents.at(i)->debug_showGoal();
		}

		
		ExplorationManager::getInstance()->printInfo();
		/// @todo Commander::getInstance()->printInfo();

		if (mDebugLevel >= 3) CoverMap::getInstance()->debug();
		if (mDebugLevel >= 2) ResourceManager::getInstance()->printInfo();

		/// @todo Commander::getInstance()->debug_showGoal();

		if (mDebugLevel >= 1) {
			drawTerrainData();
		}
	}
}

void BatsModule::drawTerrainData() const {
	//we will iterate through all the base locations, and draw their outlines.
	std::set<BWTA::BaseLocation*>::const_iterator baseLocationIt;
	for(baseLocationIt = BWTA::getBaseLocations().begin();
		baseLocationIt != BWTA::getBaseLocations().end();
		++baseLocationIt)
	{
		TilePosition p=(*baseLocationIt)->getTilePosition();
		Position c=(*baseLocationIt)->getPosition();

		//Draw a progress bar at each resource
		std::set<BWAPI::Unit*>::const_iterator resourceIt;
		for(resourceIt = (*baseLocationIt)->getStaticMinerals().begin();
			resourceIt != (*baseLocationIt)->getStaticMinerals().end();
			++resourceIt)
		{
			if ((*resourceIt)->getResources() > 0) {
				int total = (*resourceIt)->getInitialResources();
				int done = (*resourceIt)->getResources();

				int width = 60;
				//int h = 64;

				Position startPos = Position((*resourceIt)->getPosition().x() - width/2 + 2,
					(*resourceIt)->getPosition().y() - 4);
				Position endPos = Position(startPos.x() + width, startPos.y() + 8);
				int progress = (int)((double)done / (double)total * width);
				Position progressPart = Position(startPos.x() + progress, startPos.y() +  8);

				Broodwar->drawBox(CoordinateType::Map, startPos.x(), startPos.y(),
					endPos.x(), endPos.y(),	Colors::Orange, false);
				Broodwar->drawBox(CoordinateType::Map, startPos.x(), startPos.y(),
					progressPart.x(), progressPart.y(),	Colors::Orange, true);
			}
		}
	}

	if (mDebugLevel >= 2)
	{
		//we will iterate through all the regions and draw the polygon outline of it in white.
		std::set<BWTA::Region*>::const_iterator regionIt;
		for(regionIt = BWTA::getRegions().begin();
			regionIt != BWTA::getRegions().end();
			++regionIt) {

			BWTA::Polygon polygon=(*regionIt)->getPolygon();
			for(int polygonPoint = 0; polygonPoint < (int)polygon.size(); polygonPoint++)
			{
				Position currentPoint = polygon[polygonPoint];
				Position nextPoint = polygon[(polygonPoint+1) % polygon.size()];
				Broodwar->drawLine(CoordinateType::Map,currentPoint.x(), currentPoint.y(),
					nextPoint.x(), nextPoint.y(), Colors::Brown);
			}
		}

		//we will visualize the chokepoints with yellow lines
		for(regionIt = BWTA::getRegions().begin();
			regionIt != BWTA::getRegions().end();
			++regionIt)
		{
			std::set<BWTA::Chokepoint*>::const_iterator chokepointIt;
			for(chokepointIt = (*regionIt)->getChokepoints().begin();
				chokepointIt != (*regionIt)->getChokepoints().end();
				++chokepointIt)
			{
				Position point1 = (*chokepointIt)->getSides().first;
				Position point2 = (*chokepointIt)->getSides().second;
				Broodwar->drawLine(CoordinateType::Map, point1.x(), point1.y(),
					point2.x(), point2.y(), Colors::Yellow);
			}
		}
	}

	//locate zerg eggs and draw progress bars
	if (BuildPlanner::isZerg()) {
		std::set<Unit*>::const_iterator unitIt;
		for(unitIt = Broodwar->self()->getUnits().begin();
			unitIt != Broodwar->self()->getUnits().end();
			++unitIt)
		{
			if ((*unitIt)->getType().getID() == UnitTypes::Zerg_Egg.getID() ||
				(*unitIt)->getType().getID() == UnitTypes::Zerg_Lurker_Egg.getID()
				|| (*unitIt)->getType().getID() == UnitTypes::Zerg_Cocoon.getID())
			{
				int total = (*unitIt)->getBuildType().buildTime();
				int done = total - (*unitIt)->getRemainingBuildTime();

				int width = (*unitIt)->getType().tileWidth() * 32;
				//int height = (*unitIt)->getType().tileHeight() * 32;

				Position startPos = Position((*unitIt)->getPosition().x() - width/2,
					(*unitIt)->getPosition().y() - 4);
				Position endPos = Position(startPos.x() + width, startPos.y() + 8);
				int progress = (int)((double)done / (double)total * width);
				Position progressPos = Position(startPos.x() + progress, startPos.y() +  8);

				Broodwar->drawBox(CoordinateType::Map, startPos.x(), startPos.y(),
					endPos.x(), endPos.y(), Colors::Blue, false);
				Broodwar->drawBox(CoordinateType::Map, startPos.x(), startPos.y(),
					progressPos.x(), progressPos.y(), Colors::Blue, true);
			}
		}
	}
}