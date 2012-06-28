#include "BatsModule.h"
#include "BuildPlanner.h"
#include "UnitCreator.h"
#include "Commander.h"
#include "Helper.h"
#include "SquadManager.h"
#include "UnitManager.h"
#include "GameTime.h"
#include "ExplorationManager.h"
#include "ResourceCounter.h"
#include "AttackCoordinator.h"
#include "WaitGoalManager.h"
#include "AlliedArmyManager.h"
#include "BTHAIModule/Source/FileReaderUtils.h"
#include "BTHAIModule/Source/AgentManager.h"
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

#define TEST_SELF() if (Broodwar->self() == NULL) {\
	ERROR_MESSAGE(false, "self() is NULL!"); \
	return; \
}

BatsModule::BatsModule() : BTHAIModule() {
	mpProfiler = NULL;
	mpUnitManager = NULL;
	mpCommander = NULL;
	mpResourceCounter = NULL;
	mpExplorationManager = NULL;
	mpWaitGoalManager = NULL;
	mpSquadManager = NULL;
	mpGameTime = NULL;
	mpAlliedArmyManager = NULL;

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
	TEST_SELF();

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

	DEBUG_MESSAGE(utilities::LogLevel_Info, "BATS bot running!");

	running = true;

	mpProfiler->end("OnInit");
}

void BatsModule::onEnd() {
	Pathfinder::getInstance()->stop();
	mpProfiler->dumpToFile();

	releaseGameClasses();

	TEST_SELF();
}

void BatsModule::onFrame() {
	TEST_SELF();

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
	TEST_SELF();

	utilities::string::toLower(text);

	// /d# needs to be overridden because base class calls a class we don't initialize
	if (startsWith(text, "/d") || startsWith(text, "/debug ")) {

		if (text == "/d1" || text == "/debug graphics low") {
			config::debug::GRAPHICS_VERBOSITY = config::debug::GraphicsVerbosity_Low;
		} else if (text == "/d2" || text == "/debug graphics medium") {
			config::debug::GRAPHICS_VERBOSITY = config::debug::GraphicsVerbosity_Medium;
		} else if (text == "/d3" || text == "/debug graphics high") {
			config::debug::GRAPHICS_VERBOSITY = config::debug::GraphicsVerbosity_High;
		} else if (text == "/d0" || text == "/debug graphics off") {
			config::debug::GRAPHICS_VERBOSITY = config::debug::GraphicsVerbosity_Off;
		} else if (startsWith(text, "/debug message")) {
			if (text == "/debug message finest") {
				utilities::setVerbosityLevel(utilities::LogLevel_Finest, OUTPUT_STARCRAFT);
			} else if (text == "/debug message finer") {
				utilities::setVerbosityLevel(utilities::LogLevel_Finer, OUTPUT_STARCRAFT);
			} else if (text == "/debug message fine") {
				utilities::setVerbosityLevel(utilities::LogLevel_Fine, OUTPUT_STARCRAFT);
			} else if (text == "/debug message info") {
				utilities::setVerbosityLevel(utilities::LogLevel_Info, OUTPUT_STARCRAFT);
			} else if (text == "/debug message warning") {
				utilities::setVerbosityLevel(utilities::LogLevel_Warning, OUTPUT_STARCRAFT);
			} else if (text == "/debug message severe") {
				utilities::setVerbosityLevel(utilities::LogLevel_Severe, OUTPUT_STARCRAFT);
			} else if (text == "/debug message off") {
				utilities::setVerbosityLevel(utilities::LogLevel_Off, OUTPUT_STARCRAFT);
			} else {
				Broodwar->printf("Invalid message value. Valid message values are:\noff\nsevere\nwarning\ninfo\nfine\nfiner\nfinest");
			}
		} else if (startsWith(text, "/debug graphics")) {
			Broodwar->printf("Invalid graphics value. Valid graphics values are:\noff\nhigh\nmedium\nlow");
		} else {
			Broodwar->printf("Invalid command, valid debug modes are\n/debug message|graphics value\nSee \"/debug message\" or \"/debug graphics\" for valid values.");
		}

	} else if (text == "/transition" || text == "transition") {
		BuildPlanner::getInstance()->switchToPhase("");
	} else if (startsWith(text,"/transition")) {				
		BuildPlanner::getInstance()->switchToPhase(text.substr(12, text.length()-12));
	} else if (mpCommander->isCommandAvailable(text)) {
		mpCommander->issueCommand(text);
	} else if (text == "/reload config") {
		config::loadConfig();
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Configuration reloaded");
	} else {
		// Default behavior
		BTHAIModule::onSendText(text);
	}
}

bool BatsModule::startsWith(const std::string& text,const std::string& token) {
	
	if(text.length() < token.length() || text.length() == 0 || token.length() == 0)
		return false;

	for(unsigned int i=0; i<token.length(); ++i)
	{
		if(text[i] != token[i])
			return false;
	}

	return true;
}


void BatsModule::onPlayerLeft(BWAPI::Player* player) {
	TEST_SELF();

	// Stop game if we left the game
	if (player == Broodwar->self()) {
		onEnd();
	}
	// Print out the player that left the game
	else {
		Broodwar->sendText("%s left the game.",player->getName().c_str());
	}
}

void BatsModule::onUnitShow(BWAPI::Unit* pUnit) {
	TEST_SELF();

	// Only add enemy units to exploration manager
	if (isEnemy(pUnit)) {
		mpExplorationManager->addSpottedUnit(pUnit);
	} else if (isAllied(pUnit)) {
		mpAlliedArmyManager->addUnit(pUnit);

		DEBUG_MESSAGE(utilities::LogLevel_Finer, "Allied unit showed: " <<
			pUnit->getType().getName());
	}
}

void BatsModule::onUnitCreate(BWAPI::Unit* pUnit) {
	TEST_SELF();

	if (areWePlaying()) {

		if (isOurs(pUnit)) {
			mpUnitManager->addAgent(pUnit);

			// Remove from build order if it's a building
			if (pUnit->getType().isBuilding()) {
				BuildPlanner::getInstance()->unlock(pUnit->getType());
			}
		}
	}
}

void BatsModule::onUnitDestroy(BWAPI::Unit* pUnit) {
	TEST_SELF();

	if (areWePlaying()) {

		if (isOurs(pUnit)) {
			mpUnitManager->removeAgent(pUnit);			
			if (pUnit->getType().isBuilding()) {
				BuildPlanner::getInstance()->buildingDestroyed(pUnit);
			}
			UnitCreator::getInstance()->updatePopulation(pUnit->getType());
			// Assist workers under attack
			if (pUnit->getType().isWorker()) {
				/// @todo assist worker.
				/// Commander::getInstance()->assistWorker(mpAgentManager->getAgent(pUnit->getID()));
			}

			mpUnitManager->cleanup();
		}
		// Enemies
		else if (isEnemy(pUnit)) {
			mpExplorationManager->unitDestroyed(pUnit);
		}
		// Allied
		else if (isAllied(pUnit)) {
			mpAlliedArmyManager->removeUnit(pUnit);

			DEBUG_MESSAGE(utilities::LogLevel_Finer, "Allied unit destroyed: " <<
				pUnit->getType().getName());
		}
	}
}

void BatsModule::onUnitMorph(BWAPI::Unit* pUnit) {
	TEST_SELF();

	if (areWePlaying()) {
		if (isOurs(pUnit)) {
			if (BuildPlanner::isZerg()) {
				mpUnitManager->morphDrone(pUnit);
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
		DEBUG_MESSAGE(utilities::LogLevel_Info, "No workers left; bailing out.");
		Broodwar->sendText("gg");
		Broodwar->leaveGame();
		return;
	}

	mpGameTime->update();
	mpResourceCounter->update();
	mpWaitGoalManager->update();

	mpAlliedArmyManager->update();

	mpUnitManager->computeActions();
	BuildPlanner::getInstance()->computeActions();
	mpCommander->computeActions();
	mpExplorationManager->computeActions();
}

void BatsModule::initGameClasses() {
	mpGameTime = GameTime::getInstance();
	mpWaitGoalManager = WaitGoalManager::getInstance();
	mpSquadManager = SquadManager::getInstance();
	AttackCoordinator::getInstance();
	mpResourceCounter = ResourceCounter::getInstance();
	mpUnitManager = UnitManager::getInstance();
	mpCommander = Commander::getInstance();
	CoverMap::getInstance();
	BuildPlanner::getInstance();
	UnitCreator::getInstance();
	UpgradesPlanner::getInstance();
	ResourceManager::getInstance();
	Pathfinder::getInstance();
	mpExplorationManager = ExplorationManager::getInstance();
	mpAlliedArmyManager = AlliedArmyManager::getInstance();
}

void BatsModule::releaseGameClasses() {
	SAFE_DELETE(mpAlliedArmyManager);
	SAFE_DELETE(mpExplorationManager);
	delete Pathfinder::getInstance();
	delete ResourceManager::getInstance();
	delete UpgradesPlanner::getInstance();
	delete BuildPlanner::getInstance();
	delete CoverMap::getInstance();
	SAFE_DELETE(mpCommander);
	SAFE_DELETE(mpUnitManager);
	SAFE_DELETE(mpResourceCounter);
	delete AttackCoordinator::getInstance();
	SAFE_DELETE(mpSquadManager);
	SAFE_DELETE(mpWaitGoalManager);
	SAFE_DELETE(mpGameTime);
}

void BatsModule::showDebug() const {
	BuildPlanner::getInstance()->printInfo();
	if (config::debug::GRAPHICS_VERBOSITY != config::debug::GraphicsVerbosity_Off) {
		mpAlliedArmyManager->printInfo();

		// Low
		/// @todo Commander::getInstance()->printInfo();
		/// /// @todo Commander::getInstance()->debug_showGoal();
		ExplorationManager::getInstance()->printInfo();
		drawTerrainData();

		// Only low
		if(config::debug::GRAPHICS_VERBOSITY == config::debug::GraphicsVerbosity_Low) {
			UnitCreator::getInstance()->printInfo();
		}

				std::vector<BaseAgent*> agents = mpUnitManager->getAgents();

		// Agents
		for (int i = 0; i < (int)agents.size(); i++) {
			// Low while building
			if (agents.at(i)->isBuilding()) {
				agents.at(i)->debug_showGoal();
			}
			// Medium if not building
			if (!agents.at(i)->isBuilding() &&
				config::debug::GRAPHICS_VERBOSITY_IN_DEBUG >= config::debug::GraphicsVerbosity_Medium)
			{
				agents.at(i)->debug_showGoal();
			}
		}


		// Medium
		if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
			ResourceManager::getInstance()->printInfo();
		}

		
		// High
		if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_High) {
			CoverMap::getInstance()->debug();
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

	
	// Medium
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium)
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