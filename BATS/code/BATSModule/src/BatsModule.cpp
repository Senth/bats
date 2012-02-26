#include "BatsModule.h"
#include "Helper.h"
#include "BTHAIModule/Source/AgentManager.h"
#include "BTHAIModule/Source/BuildPlanner.h"
#include "BTHAIModule/Source/ExplorationManager.h"
#include "BTHAIModule/Source/CoverMap.h"
#include "BTHAIModule/Source/Commander.h"
#include "BTHAIModule/Source/PathFinder.h"
#include "BTHAIModule/Source/UpgradesPlanner.h"
#include "BTHAIModule/Source/ResourceManager.h"
#include "BTHAIModule/Source/Profiler.h"
#include "BTHAIModule/Source/Config.h"
#include "Utilities/Helper.h"
#include <cassert>

using namespace bats;
using namespace BWAPI;

const int WORKER_MINERAL_PRICE = 50;
const int GAME_STARTED_FRAME = 1;

BatsModule::BatsModule() : BTHAIModule() {
	mpProfiler = NULL;
	mpAgentManager = NULL;

	// Initialize singletons
	mpProfiler = Profiler::getInstance();
}

BatsModule::~BatsModule() {
	SAFE_DELETE(mpProfiler);

	// Release game classes, if the game was ended abruptly onEnd() might not have been called.
	releaseGameClasses();
}

void BatsModule::onStart() {
	mpProfiler->start("OnInit");

	Broodwar->enableFlag(BWAPI::Flag::UserInput);
	// Set default speed
	speed = 8;
	Broodwar->setLocalSpeed(speed);

	BWTA::readMap();
	BWTA::analyze();
	//AnalyzeThread();

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
			mpAgentManager->addAgent(*unitIt);
		}
	}

	//Broodwar->printf("BTHAI %s (%s)", VERSION.c_str(), Broodwar->self()->getRace().getName().c_str());

	running = true;

	mpProfiler->end("OnInit");
}

void BatsModule::onEnd() {
	//Pathfinder::getInstance()->stop();
	//mpProfiler->dumpToFile();

	//releaseGameClasses();
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

	Config::getInstance()->displayBotName();

	mpProfiler->end("OnFrame");

	// Show profiler information if profiling is on
	if (profile) {
		mpProfiler->showAll();
	}
}

void BatsModule::onSendText(std::string text) {

	// /d# needs to be over-riden because base class calls a class we don't initialize
	if (text == "/d1" || text == "/debug low") {
		mDebugLevel = 1;
	} else if (text == "/d2" || text == "/debug medium") {
		mDebugLevel = 2;
	} else if (text == "/d3" || text == "/debug high") {
		mDebugLevel = 3;
	} else if (text == "/d0" || text == "/debug off") {
		mDebugLevel = 4;
	} else {
		// Default behavior
		BTHAIModule::onSendText(text);
	}
}

void BatsModule::onUnitCreate(BWAPI::Unit* pUnit) {
	if (areWePlaying()) {
		mpAgentManager->addAgent(pUnit);

		// Remove from build order if it's a building
		if (pUnit->getType().isBuilding()) {
			BuildPlanner::getInstance()->unlock(pUnit->getType());
		}
	}
}

void BatsModule::onUnitDestroy(BWAPI::Unit* pUnit) {
	if (areWePlaying()) {

		if (OUR(pUnit)) {
			mpAgentManager->removeAgent(pUnit);

			if (pUnit->getType().isBuilding()) {
				BuildPlanner::getInstance()->buildingDestroyed(pUnit);
			}

			// Assist workers under attack
			if (pUnit->getType().isWorker()) {
				Commander::getInstance()->assistWorker(mpAgentManager->getAgent(pUnit->getID()));
			}

			mpAgentManager->cleanup();
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
	const std::vector<BaseAgent*>& agents = mpAgentManager->getAgents();
	std::vector<BaseAgent*>::const_iterator agentIt = agents.begin();
	bool attackingUnitExist = false;
	while(!attackingUnitExist && agentIt != agents.end()) {
		if ((*agentIt)->isAttacking()) {
			attackingUnitExist = true;
		}

		++agentIt;
	}

	if (mpAgentManager->getNoWorkers() == 0 &&
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

	mpAgentManager->computeActions();
	BuildPlanner::getInstance()->computeActions();
	Commander::getInstance()->computeActions();
	ExplorationManager::getInstance()->computeActions();
}

void BatsModule::initGameClasses() {
	CoverMap::getInstance();
	BuildPlanner::getInstance();
	UpgradesPlanner::getInstance();
	ResourceManager::getInstance();
	Pathfinder::getInstance();
	mpAgentManager = AgentManager::getInstance();
}

void BatsModule::releaseGameClasses() {
	//delete CoverMap::getInstance();
	//delete BuildPlanner::getInstance();
	//delete UpgradesPlanner::getInstance();
	//delete ResourceManager::getInstance();
	//delete Pathfinder::getInstance();
	//SAFE_DELETE(mpAgentManager);
}

void BatsModule::showDebug() const {
	if (mDebugLevel > 0) {
		std::vector<BaseAgent*> agents = mpAgentManager->getAgents();
		for (int i = 0; i < (int)agents.size(); i++) {
			if (agents.at(i)->isBuilding()) agents.at(i)->debug_showGoal();
			if (!agents.at(i)->isBuilding() && mDebugLevel >= 2) agents.at(i)->debug_showGoal();
		}

		BuildPlanner::getInstance()->printInfo();
		ExplorationManager::getInstance()->printInfo();
		Commander::getInstance()->printInfo();

		if (mDebugLevel >= 3) CoverMap::getInstance()->debug();
		if (mDebugLevel >= 2) ResourceManager::getInstance()->printInfo();

		Commander::getInstance()->debug_showGoal();

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