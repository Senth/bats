#include "BatsModule.h"
#include "BuildPlanner.h"
#include "UnitCreator.h"
#include "Commander.h"
#include "UnitHelper.h"
#include "Helper.h"
#include "SquadManager.h"
#include "UnitManager.h"
#include "GameTime.h"
#include "ExplorationManager.h"
#include "ResourceCounter.h"
#include "AttackCoordinator.h"
#include "WaitGoalManager.h"
#include "PlayerArmyManager.h"
#include "DefenseManager.h"
#include "SelfClassifier.h"
#include "IntentionWriter.h"
#include "BTHAIModule/Source/FileReaderUtils.h"
#include "BTHAIModule/Source/AgentManager.h"
#include "BTHAIModule/Source/CoverMap.h"
#include "BTHAIModule/Source/PathFinder.h"
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
const int GAME_STARTED_FRAME = -1;

#define TEST_SELF() if (Broodwar->self() == NULL) {\
	ERROR_MESSAGE(false, "self() is NULL!"); \
	return; \
}

BatsModule::BatsModule() : BTHAIModule() {
	mProfiler = NULL;
	mUnitManager = NULL;
	mCommander = NULL;
	mResourceCounter = NULL;
	mExplorationManager = NULL;
	mWaitGoalManager = NULL;
	mSquadManager = NULL;
	mGameTime = NULL;
	mPlayerArmyManager = NULL;
	mDefenseManager = NULL;
	mIntentionWriter = NULL;

	// Initialize logger
	utilities::setOutputDirectory(config::log::OUTPUT_DIR);
	utilities::loadLogSettings(config::log::SETTINGS_FILE);
	config::loadConfig();

	// Initialize singletons that stays throughout multiple games
	mProfiler = Profiler::getInstance();
}

BatsModule::~BatsModule() {
	mProfiler->dumpToFile();
	SAFE_DELETE(mProfiler);

	// Release game classes, if the game was ended abruptly onEnd() might not have been called.
	releaseGameClasses();

	utilities::checkForErrors();
}

void BatsModule::onStart() {
	TEST_SELF();

	mProfiler->start("OnInit");

	Broodwar->enableFlag(BWAPI::Flag::UserInput);
	// Set default speed
	std::string speed = "speed ";
	speed += utilities::string::toString(config::game::SPEED);
	Broodwar->setLocalSpeed(config::game::SPEED);
	Broodwar->sendText(speed.c_str());

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

	DEBUG_MESSAGE(utilities::LogLevel_Info, "BATS bot running!");

	running = true;

	mProfiler->end("OnInit");
}

void BatsModule::onEnd() {
	Pathfinder::getInstance()->stop();
	mProfiler->dumpToFile();

	releaseGameClasses();

	TEST_SELF();
}

void BatsModule::onFrame() {
	TEST_SELF();

	mProfiler->start("OnFrame");

	// Some states to skip further processing
	if (!running || // Game over
		!Broodwar->isInGame() ||
		Broodwar->isReplay()) {

		mProfiler->end("OnFrame");
		return;
	}

	updateGame();
	showDebug();

	mProfiler->end("OnFrame");

	// Show profiler information if profiling is on
	if (profile) {
		mProfiler->showAll();
	}
}

void BatsModule::onSendText(std::string text) {
	TEST_SELF();

	utilities::string::toLower(text);

	// /d# needs to be overridden because base class calls a class we don't initialize
	if (utilities::string::startsWith(text, "/d") || utilities::string::startsWith(text, "/debug ")) {

		if (text == "/d1" || text == "/debug graphics low") {
			config::debug::GRAPHICS_VERBOSITY = config::debug::GraphicsVerbosity_Low;
		} else if (text == "/d2" || text == "/debug graphics medium") {
			config::debug::GRAPHICS_VERBOSITY = config::debug::GraphicsVerbosity_Medium;
		} else if (text == "/d3" || text == "/debug graphics high") {
			config::debug::GRAPHICS_VERBOSITY = config::debug::GraphicsVerbosity_High;
		} else if (text == "/d0" || text == "/debug graphics off") {
			config::debug::GRAPHICS_VERBOSITY = config::debug::GraphicsVerbosity_Off;
		} else if (utilities::string::startsWith(text, "/debug message")) {
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
		} else if (utilities::string::startsWith(text, "/debug graphics")) {
			Broodwar->printf("Invalid graphics value. Valid graphics values are:\noff\nhigh\nmedium\nlow");
		} else {
			Broodwar->printf("Invalid command, valid debug modes are\n/debug message|graphics value\nSee \"/debug message\" or \"/debug graphics\" for valid values.");
		}

	} else if (text == "/transition") {
		BuildPlanner::getInstance()->switchToPhase("");
	} else if (utilities::string::startsWith(text,"/transition")) {				
		BuildPlanner::getInstance()->switchToPhase(text.substr(12, text.length()-12));
	} else if (mCommander->isCommandAvailable(text)) {
		mCommander->issueCommand(text);
	} else if (text == "/reload config") {
		config::loadConfig();
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Configuration reloaded");
	} else if (utilities::string::startsWith(text, "speed")) {
		int speed = -1;

		std::string speedValueStr = text.substr(5);
		std::stringstream ss;
		ss << speedValueStr;
		ss >> speed;

		Broodwar->setLocalSpeed(speed);
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Global speed set to: " << speed);
		Broodwar->sendText(text.c_str());
	} else if (utilities::string::startsWith(text, "test")) {
		std::string value = text.substr(5);

		// Test something, this code below can be changed freely by whomever 
		if (value == "supply") {
			BuildPlanner::getInstance()->addItemFirst(UnitTypes::Terran_Supply_Depot);
		} else if (value == "academy") {
			BuildPlanner::getInstance()->addItemFirst(UnitTypes::Terran_Academy);
		}
	} else {
		// Default behavior
		Broodwar->sendText(text.c_str());
	}
}

#pragma warning(push)
#pragma warning(disable:4100)
void BatsModule::onReceiveText(BWAPI::Player* pPlayer, std::string text) {
	TEST_SELF();

	utilities::string::toLower(text);

	if (utilities::string::startsWith(text, "speed")) {
		int speed = -1;

		std::string speedValueStr = text.substr(5);
		std::stringstream ss;
		ss << speedValueStr;
		ss >> speed;

		Broodwar->setLocalSpeed(speed);
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Received speed message, speed set to: " << speed);
	} else if (mCommander->isCommandAvailable(text)) {
		DEBUG_MESSAGE(utilities::LogLevel_Fine, "Command message received: " << text);
		mCommander->issueCommand(text);
	} else {
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Received message: " << text);
	}
}
#pragma warning(pop)

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

#pragma warning(push)
#pragma warning(disable:4100)
void BatsModule::onNukeDetect(BWAPI::Position target) {
	/// @todo implement nuke detection, and ping the minimap
}
#pragma warning(pop)

void BatsModule::onUnitDiscover(BWAPI::Unit* unit) {
	TEST_SELF();

	if (areWePlaying()) {

		if (UnitHelper::isOurs(unit)) {
			mUnitManager->addAgent(unit);

			// Remove from build order if it's a building
			if (unit->getType().isBuilding()) {
				BuildPlanner::getInstance()->removeFromQueue(unit->getType());
			}
		} else if (UnitHelper::isEnemy(unit)) {
			mExplorationManager->addSpottedUnit(unit);
			mPlayerArmyManager->addUnit(unit);
		} else if (UnitHelper::isAllied(unit)) {
			mPlayerArmyManager->addUnit(unit);

			DEBUG_MESSAGE(utilities::LogLevel_Finer, "Allied unit showed: " <<
				unit->getType().getName());
		}
	}
}

void BatsModule::onUnitEvade(BWAPI::Unit* unit) {
	TEST_SELF();

	if (areWePlaying()) {

		if (UnitHelper::isOurs(unit)) {
			mUnitManager->removeAgent(unit);			
			if (unit->getType().isBuilding()) {
				BuildPlanner::getInstance()->buildingDestroyed(unit);
			}
			else
				UnitCreator::getInstance()->updatePopulation(unit->getType());
			// Assist workers under attack
			if (unit->getType().isWorker()) {
				/// @todo assist worker.
				/// Commander::getInstance()->assistWorker(mpAgentManager->getAgent(unit->getID()));
			}

			mUnitManager->cleanup();
		}
		// Enemies
		else if (UnitHelper::isEnemy(unit)) {
			mPlayerArmyManager->removeUnit(unit);
		}
		// Allied
		else if (UnitHelper::isAllied(unit)) {
			mPlayerArmyManager->removeUnit(unit);

			DEBUG_MESSAGE(utilities::LogLevel_Finer, "Allied unit destroyed: " <<
				unit->getType().getName());
		}
	}
}

#pragma warning(push)
#pragma warning(disable:4100)
void BatsModule::onUnitShow(BWAPI::Unit* unit) {
	// Does nothing, handled in onUnitDiscover()
}

void BatsModule::onUnitHide(BWAPI::Unit* unit) {
	// Does nothing, handled in onUnitEvade()
}

void BatsModule::onUnitCreate(BWAPI::Unit* unit) {
	// Does nothing, handled in onUnitDiscover()
}
#pragma warning(pop)

void BatsModule::onUnitDestroy(BWAPI::Unit* unit) {
	TEST_SELF();

	if (UnitHelper::isEnemy(unit)) {
		mExplorationManager->unitDestroyed(unit);
	}
}

void BatsModule::onUnitMorph(BWAPI::Unit* unit) {
	TEST_SELF();

	if (areWePlaying()) {
		if (UnitHelper::isOurs(unit)) {
			if (BuildPlanner::isZerg()) {
				mUnitManager->morphDrone(unit);
				BuildPlanner::getInstance()->removeFromQueue(unit->getType());
			} else {
				onUnitDiscover(unit);
			}
		}
	}
}

#pragma warning(push)
#pragma warning(disable:4100)
void BatsModule::onUnitRenegade(BWAPI::Unit* unit) {
	// Remove renegade units that became neutral, e.g. could be addons too when loosing the main structure
	if (UnitHelper::isNeutral(unit)) {
		mExplorationManager->unitDestroyed(unit);
		mUnitManager->removeAgent(unit);
	}
}
#pragma warning(pop)

void BatsModule::onSaveGame(std::string gameName) {
	DEBUG_MESSAGE(utilities::LogLevel_Info, "Saved game " << gameName);
}

bool BatsModule::isGameLost() const {
	// Check if we have at least one attacking unit
	const std::vector<BaseAgent*>& agents = mUnitManager->getAgents();
	std::vector<BaseAgent*>::const_iterator agentIt = agents.begin();
	bool attackingUnitExist = false;
	while(!attackingUnitExist && agentIt != agents.end()) {
		if ((*agentIt)->isAttacking()) {
			attackingUnitExist = true;
		}

		++agentIt;
	}

	if (mUnitManager->getWorkerCount() == 0 &&
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
	mProfiler->start("updateGame");

	// Quit the game if the game is lost
	if (isGameLost()) {
		DEBUG_MESSAGE(utilities::LogLevel_Info, "No workers left; bailing out.");
		Broodwar->sendText("gg");
		Broodwar->leaveGame();
		return;
	}

	mGameTime->update();
	mResourceCounter->update();
	mWaitGoalManager->update();
	mPlayerArmyManager->update();

	mExplorationManager->update();
	mSquadManager->update();
	BuildPlanner::getInstance()->computeActions();
	mCommander->computeActions();
	mDefenseManager->update();
	mUnitManager->computeActions();
	

	mProfiler->end("updateGame");
}

void BatsModule::initGameClasses() {
	mGameTime = GameTime::getInstance();
	mWaitGoalManager = WaitGoalManager::getInstance();
	mSquadManager = SquadManager::getInstance();
	AttackCoordinator::getInstance();
	mResourceCounter = ResourceCounter::getInstance();
	mUnitManager = UnitManager::getInstance();
	mCommander = Commander::getInstance();
	CoverMap::getInstance();
	BuildPlanner::getInstance();
	UnitCreator::getInstance();
	ResourceManager::getInstance();
	Pathfinder::getInstance();
	mExplorationManager = ExplorationManager::getInstance();
	mPlayerArmyManager = PlayerArmyManager::getInstance();
	mDefenseManager = DefenseManager::getInstance();
	mIntentionWriter = IntentionWriter::getInstance();
}

void BatsModule::releaseGameClasses() {
	SAFE_DELETE(mIntentionWriter);
	SAFE_DELETE(mDefenseManager);
	SAFE_DELETE(mPlayerArmyManager);
	SAFE_DELETE(mExplorationManager);
	Pathfinder::getInstance()->stop();
	delete Pathfinder::getInstance();
	delete ResourceManager::getInstance();
	delete BuildPlanner::getInstance();
	delete CoverMap::getInstance();
	SAFE_DELETE(mCommander);
	SAFE_DELETE(mUnitManager);
	SAFE_DELETE(mResourceCounter);
	delete AttackCoordinator::getInstance();
	SAFE_DELETE(mSquadManager);
	SAFE_DELETE(mWaitGoalManager);
	SAFE_DELETE(mGameTime);
	delete SelfClassifier::getInstance();
}

void BatsModule::showDebug() const {
	if (config::debug::GRAPHICS_VERBOSITY != config::debug::GraphicsVerbosity_Off) {
		mProfiler->start("printGraphicDebugInfo()");

		BuildPlanner::getInstance()->printGraphicDebugInfo();
		mPlayerArmyManager->printGraphicDebugInfo();
		mSquadManager->printGraphicDebugInfo();
		mUnitManager->printGraphicDebugInfo();
		mDefenseManager->printGraphicDebugInfo();
		mExplorationManager->printGraphicDebugInfo();
		UnitCreator::getInstance()->printGraphicDebugInfo();
		drawTerrainData();
		CoverMap::getInstance()->printGraphicDebugInfo();

		// ExplorationManager::getInstance()->printInfo();


		// Medium
		if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
			ResourceManager::getInstance()->printInfo();
		}

		mProfiler->end("printGraphicDebugInfo()");
	}
}

void BatsModule::drawTerrainData() const {
	if (config::debug::GRAPHICS_VERBOSITY == config::debug::GraphicsVerbosity_Off ||
		config::debug::modules::TERRAIN == false)
	{
		return;
	}


	// Low
	// Draw resource bars. High -> draw resource group id
	// Zerg egg progress
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Low) {

		//Draw a progress bar at each resource
		std::set<BWAPI::Unit*> resources = Broodwar->getStaticMinerals();
		resources.insert(Broodwar->getStaticGeysers().begin(), Broodwar->getStaticGeysers().end());
		std::set<BWAPI::Unit*>::const_iterator resourceIt;
		for(resourceIt = resources.begin(); resourceIt != resources.end(); ++resourceIt) {
			int resourceWidth = 60;
			if ((*resourceIt)->getResources() > 0) {
				int total = (*resourceIt)->getInitialResources();
				int done = (*resourceIt)->getResources();

				Position startPos = Position((*resourceIt)->getPosition().x() - resourceWidth / 2 + 2,
					(*resourceIt)->getPosition().y() - 4);
				Position endPos = Position(startPos.x() + resourceWidth, startPos.y() + 8);
				int progress = (int)((double)done / (double)total * resourceWidth);
				Position progressPart = Position(startPos.x() + progress, startPos.y() +  8);

				Broodwar->drawBoxMap(startPos.x(), startPos.y(),
					endPos.x(), endPos.y(),	Colors::Orange, false);
				Broodwar->drawBoxMap(startPos.x(), startPos.y(),
					progressPart.x(), progressPart.y(),	Colors::Orange, true);

				// High -> Draw resource group
				if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_High) {
					int resourceGroupId = (*resourceIt)->getResourceGroup();
					const Position& resourcePos = (*resourceIt)->getInitialPosition();
					Broodwar->drawTextMap(
						resourcePos.x() - resourceWidth / 2 + 2,
						resourcePos.y() + 4,
						"%sGroup: %i",
						TextColors::ORANGE.c_str(),
						resourceGroupId);
				}
			}
		}

		//locate Zerg eggs and draw progress bars
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

					Broodwar->drawBoxMap(startPos.x(), startPos.y(),
						endPos.x(), endPos.y(), Colors::Blue, false);
					Broodwar->drawBoxMap(startPos.x(), startPos.y(),
						progressPos.x(), progressPos.y(), Colors::Blue, true);
				}
			}
		}
	}

	
	// Medium
	// Draw outlines for regions 
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
		//we will iterate through all the regions and draw the polygon outline of it in white.
		std::set<BWTA::Region*>::const_iterator regionIt;
		for(regionIt = BWTA::getRegions().begin();
			regionIt != BWTA::getRegions().end();
			++regionIt) {

			BWTA::Polygon polygon=(*regionIt)->getPolygon();
			for(int polygonPoint = 0; polygonPoint < (int)polygon.size(); ++polygonPoint) {
				Position currentPoint = polygon[polygonPoint];
				Position nextPoint = polygon[(polygonPoint+1) % polygon.size()];
				Broodwar->drawLineMap(currentPoint.x(), currentPoint.y(),
					nextPoint.x(), nextPoint.y(), Colors::Brown);
			}
		}

		//we will visualize the chokepoints with yellow lines
		for(regionIt = BWTA::getRegions().begin(); regionIt != BWTA::getRegions().end(); ++regionIt) {
			std::set<BWTA::Chokepoint*>::const_iterator chokepointIt;
			for(chokepointIt = (*regionIt)->getChokepoints().begin();
				chokepointIt != (*regionIt)->getChokepoints().end();
				++chokepointIt)
			{
				Position point1 = (*chokepointIt)->getSides().first;
				Position point2 = (*chokepointIt)->getSides().second;
				Broodwar->drawLineMap(point1.x(), point1.y(),
					point2.x(), point2.y(), Colors::Yellow);
			}
		}
	}
}