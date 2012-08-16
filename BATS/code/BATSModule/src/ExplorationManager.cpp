#include "ExplorationManager.h"
#include "UnitManager.h"
//#include "VultureAgent.h"
#include "Commander.h"
#include "Squad.h"
#include "BTHAIModule/Source/CoverMap.h"
#include "BTHAIModule/Source/UnitAgent.h"
#include "BTHAIModule/Source/SpottedObject.h"
#include "BTHAIModule/Source/Profiler.h"
#include "Config.h"
#include "Helper.h"
#include <set>
#include <algorithm>
#include <cfloat>

using namespace BWAPI;
using namespace BWTA;
using namespace std;
using std::tr1::shared_ptr;
using namespace bats;

ExplorationManager* ExplorationManager::msInstance = NULL;

const double CLOSE_BASE_DISTANCE = 12.0;

ExplorationManager::ExplorationManager() {
	mForceOwn.reset();
	mForceEnemy.reset();
	mCalcTurnCurrent = CalcTurn_First;

	// Add the regions for this map
	for(set<BWTA::Region*>::const_iterator it=getRegions().begin(); it != getRegions().end(); ++it) {
		mExploreData.push_back(ExploreData(TilePosition((*it)->getCenter())));
	}

	// Add all expansions
	const set<BWTA::BaseLocation*>& baseLocations = BWTA::getBaseLocations();
	set<BWTA::BaseLocation*>::const_iterator baseLocationIt;
	for (baseLocationIt = baseLocations.begin(); baseLocationIt != baseLocations.end(); ++baseLocationIt) {
		TilePosition basePosition = (*baseLocationIt)->getTilePosition();
		mExploreData.push_back(ExploreData(basePosition, true));
	}

	mFrameLastCall = Broodwar->getFrameCount();
}

ExplorationManager::~ExplorationManager() {
	msInstance = NULL;
}

ExplorationManager* ExplorationManager::getInstance() {
	if (msInstance == NULL) {
		msInstance = new ExplorationManager();
	}
	return msInstance;
}

void ExplorationManager::update() {
	//Don't call too often
	int cFrame = Broodwar->getFrameCount();
	if (cFrame - mFrameLastCall < config::frame_distribution::EXPLORATION_MANAGER) {
		return;
	}
	mFrameLastCall = cFrame;
	
	Profiler::getInstance()->start("ExplorationManager::update()");

	// Use a rotation algorithm to calculate enemy, our, and player forces every 3 third time.
	switch (mCalcTurnCurrent) {
	case CalcTurn_Enemy:
		calcEnemyForceData();
		break;
	case CalcTurn_Our:
		calcOwnForceData();
		break;
	case CalcTurn_Player:
		/// @todo add teammate player forces
		break;
	}

	mCalcTurnCurrent = static_cast<CalcTurns>(static_cast<int>(mCalcTurnCurrent) + 1);
	if (mCalcTurnCurrent == CalcTurn_Lim) {
		mCalcTurnCurrent = CalcTurn_First;
	}


	// Check if an exploration point is visible, if it is, update visited time
	for (size_t i = 0; i < mExploreData.size(); ++i) {
		if (Broodwar->isVisible(mExploreData[i].getCenterPosition())) {
			mExploreData[i].updateVisited();
		}
	}


	// Remove dead buildings
	cleanup();


	/// @todo update position on visible buildings that have moved.

	Profiler::getInstance()->end("ExplorationManager::update()");
}

TilePosition ExplorationManager::getNextToExplore(const std::tr1::shared_ptr<Squad>& squad) {
	// Sort the exploration data, one that hasn't been explored for furthest time.
	std::sort(mExploreData.begin(), mExploreData.end());


	TilePosition currentPos = squad->getCenter();
	TilePosition goal = TilePositions::Invalid;
	

	/// @todo check for close regions first when scouting


	// Get the first in the queue that we can reach.
	// GROUND
	if (squad->travelsByGround()) {
		vector<ExploreData>::iterator exploreIt = mExploreData.begin();
		while (goal == TilePositions::Invalid && exploreIt != mExploreData.end()) {

			if (BWTA::isConnected(currentPos, exploreIt->getCenterPosition())) {
				goal = exploreIt->getCenterPosition();

				// Update visited so that we choose another one if we fail (because of enemy units)
				// Or so another squad doesn't go there.
				exploreIt->updateVisited();
			}

			++exploreIt;
		}
	}
	// AIR
	else {
		if (!mExploreData.empty()) {
			goal = mExploreData.front().getCenterPosition();

			// Update visited so that we choose another one if we fail (because of enemy units)
			// Or so another squad doesn't go there.
			mExploreData.front().updateVisited();
		}
	}

	return goal;
}

void ExplorationManager::showIntellData() const {
	Broodwar->drawTextScreen(250,16*2, "AirAttackStr: %d (%d)", mForceEnemy.airAttackStr, mForceOwn.airAttackStr);
	Broodwar->drawTextScreen(250,16*3, "AirDefendStr: %d (%d)", mForceEnemy.airDefendStr, mForceOwn.airDefendStr);
	Broodwar->drawTextScreen(250,16*4, "GroundAttackStr: %d (%d)", mForceEnemy.groundAttackStr, mForceOwn.groundAttackStr);
	Broodwar->drawTextScreen(250,16*5, "GroundDefendStr: %d (%d)", mForceEnemy.groundDefendStr, mForceOwn.groundDefendStr);

	Broodwar->drawTextScreen(250,16*6, "CommandCenters: %d (%d)", mForceEnemy.cCommandCenters, mForceOwn.cCommandCenters);
	Broodwar->drawTextScreen(250,16*7, "Factories: %d (%d)", mForceEnemy.cFactories, mForceOwn.cFactories);
	Broodwar->drawTextScreen(250,16*8, "Airports: %d (%d)", mForceEnemy.cAirports, mForceOwn.cAirports);
	Broodwar->drawTextScreen(250,16*9, "DefenseStructures: %d (%d)", mForceEnemy.cDefenseStructures, mForceOwn.cDefenseStructures);
	Broodwar->drawTextScreen(250,16*10, "DetectorStructures: %d (%d)", mForceEnemy.cDetectorStructures, mForceOwn.cDetectorStructures);
	Broodwar->drawTextScreen(250,16*11, "DetectorUnits: %d (%d)", mForceEnemy.cDetectorUnits, mForceOwn.cDetectorUnits);
}

void ExplorationManager::calcOwnForceData() {
	mForceOwn.reset();

	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++) {
		if (agents[i]->isAlive()) {
			UnitType type = agents[i]->getUnitType();
			if (type.canAttack() && !type.isWorker()) {
				if (!type.isBuilding()) {
					if (type.isFlyer()) {
						mForceOwn.airAttackStr += type.destroyScore();
					} else {
						mForceOwn.groundAttackStr += type.destroyScore();
					}
				}

				if (UnitAgent::getAirRange(type) >= 0) {
					mForceOwn.airDefendStr += type.destroyScore();
				}
				if (UnitAgent::getGroundRange(type) >= 0) {
					mForceOwn.groundDefendStr += type.destroyScore();
				}
			}

			mForceOwn.checkType(type);
		}
	}
}

void ExplorationManager::calcEnemyForceData() {
	mForceEnemy.reset();

	for (size_t i = 0; i < mSpottedUnits.size(); i++) {
		if (mSpottedUnits[i]->isActive()) {
			UnitType type = mSpottedUnits[i]->getType();
			if (type.canAttack() && !type.isWorker()) {
				if (!type.isBuilding()) {
					if (type.isFlyer()){
						mForceEnemy.airAttackStr += type.destroyScore();
					} else{
						mForceEnemy.groundAttackStr += type.destroyScore();
					}
				}

				if (UnitAgent::getAirRange(type) >= 0) {
					mForceEnemy.airDefendStr += type.destroyScore();
				}
				if (UnitAgent::getGroundRange(type) >= 0) {
					mForceEnemy.groundDefendStr += type.destroyScore();
				}
			}

			mForceEnemy.checkType(type);
		}
	}

	for (size_t i = 0; i < mSpottedStructures.size(); i++) {
		if (mSpottedStructures[i]->isActive() && mSpottedStructures[i]->getUnitID() != 10101) {
			UnitType type = mSpottedStructures[i]->getType();
			mForceEnemy.checkType(type);
		}
	}
}

void ExplorationManager::printGraphicDebugInfo() const
{
	//Uncomment this if you want to draw a mark at detected enemy buildings.
	/*for (int i = 0; i < (int)spottedBuildings.size(); i++)
	{
		if (spottedBuildings[i]->isActive())
		{
			int x1 = spottedBuildings[i]->getTilePosition().x() * 32;
			int y1 = spottedBuildings[i]->getTilePosition().y() * 32;
			int x2 = x1 + 32;
			int y2 = y1 + 32;

			Broodwar->drawBox(CoordinateType::Map,x1,y1,x2,y2,Colors::Blue,true);
		}
	}*/

	//Draw a circle around detectors
}

void ExplorationManager::addSpottedUnit(const BWAPI::Unit* unit) {
	if (unit->getType().isBuilding()) {
		
		//Check if we already have seen this building
		bool found = false;
		for (size_t i = 0; i < mSpottedStructures.size(); i++) {
			if (mSpottedStructures[i]->getUnitID() == unit->getID()) {
				found = true;
				break;
			}
		}

		if (!found) {
			//Broodwar->printf("[EM] Enemy %s spotted at (%d,%d)", unit->getType().getName().c_str(), unit->getPosition().x(), unit->getPosition().y());
			mSpottedStructures.push_back(shared_ptr<SpottedObject>(new SpottedObject(unit)));
		}
	} else{
		bool found = false;
		for (size_t i = 0; i < mSpottedUnits.size(); i++) {
			if (mSpottedUnits[i]->getUnitID() == unit->getID()) {
				found = true;
				break;
			}
		}

		if (!found) {
			mSpottedUnits.push_back(shared_ptr<SpottedObject>(new SpottedObject(unit)));
		}
	}
}

void ExplorationManager::unitDestroyed(const BWAPI::Unit* unit) {
	TilePosition unitPos = unit->getTilePosition();
	if (unit->getType().isBuilding()) {
		bool removed = false;
		std::vector<shared_ptr<SpottedObject>>::iterator it;
		it = mSpottedStructures.begin();
		while (!removed && it != mSpottedStructures.end()) {
			if ((*it)->getUnitID() == unit->getID()) {
				removed = true;
				it = mSpottedStructures.erase(it);
			} else {
				++it;
			}
		}

		DEBUG_MESSAGE_CONDITION(!removed, utilities::LogLevel_Warning,
			"ExplorationManager::unitDestroyed() | Building " << unit->getType().getName() <<
			" at (" << unitPos.x() << "," << unitPos.y() << ") was not removed!");
	} else {
		bool removed = false;
		std::vector<shared_ptr<SpottedObject>>::iterator it;
		it = mSpottedUnits.begin();
		while (!removed && it != mSpottedUnits.end()) {
			if ((*it)->getUnitID() == unit->getID()) {
				removed = true;
				it = mSpottedUnits.erase(it);
			} else {
				++it;
			}
		}
	}
}

void ExplorationManager::cleanup() {
	// Remove buildings that have been moved or destroyed
	vector<shared_ptr<SpottedObject>>::iterator structureIt = mSpottedStructures.begin();

	while (structureIt != mSpottedStructures.end()) {
		if (Broodwar->isVisible((*structureIt)->getTilePosition())) {
			Unit* pStructureUnit = Broodwar->getUnit((*structureIt)->getUnitID());

			// Remove buildings that doesn't exist
			if (pStructureUnit == NULL && !pStructureUnit->exists()) {
				structureIt = mSpottedStructures.erase(structureIt);
			} else {
				++structureIt;
			}
		} else {
			++structureIt;
		}
	}
}

int ExplorationManager::countSpottedBuildingsWithinRange(const BWAPI::TilePosition& position, int range) const {
	int cStructuresInRange = 0;
	int rangeSquared = range * range;
	for (size_t i = 0; i < mSpottedStructures.size(); i++) {
		if (mSpottedStructures[i]->isActive()) {
			int structureDistanceSquared = getSquaredDistance(mSpottedStructures[i]->getTilePosition(), position);
			if (structureDistanceSquared <= rangeSquared) {
				cStructuresInRange++;
			}
		}
	}

	return cStructuresInRange;
}

bool ExplorationManager::hasSpottedBuildingWithinRange(const BWAPI::TilePosition& position, int range) const {
	int rangeSquared = range * range;
	for (size_t i = 0; i < mSpottedStructures.size(); ++i) {
		if (mSpottedStructures[i]->isActive()) {
			int structureDistanceSquared = getSquaredDistance(mSpottedStructures[i]->getTilePosition(), position);
			if (structureDistanceSquared <= rangeSquared) {
				return true; // QUICK RETURN
			}
		}
	}

	return false;
}

std::pair<TilePosition,int> ExplorationManager::getClosestSpottedBuilding(const BWAPI::TilePosition& startPosition) const {
	TilePosition pos = BWAPI::TilePositions::Invalid;
	int bestDist = INT_MAX;

	for (size_t i = 0; i < mSpottedStructures.size(); i++) {
		if (mSpottedStructures[i]->isActive()) {
			int squaredDistance = getSquaredDistance(startPosition, mSpottedStructures[i]->getTilePosition());
			if (squaredDistance < bestDist) {
				bestDist = squaredDistance;
				pos = mSpottedStructures[i]->getTilePosition();
			}
		}
	}

	return std::make_pair(pos, bestDist);
}

vector<shared_ptr<SpottedObject>>& ExplorationManager::getSpottedBuildings() {
	return mSpottedStructures;
}

const vector<shared_ptr<SpottedObject>>& ExplorationManager::getSpottedBuildings() const {
	return mSpottedStructures;
}

bool ExplorationManager::hasSpottedBuilding() const {
	return !mSpottedStructures.empty();
}

bool ExplorationManager::canReach(const BWAPI::TilePosition& source, const BWAPI::TilePosition& destination) {
	int mapWidth = Broodwar->mapWidth();
	int mapHeight = Broodwar->mapHeight();
	if (source.x() < 0 || source.x() >= mapWidth || source.y() < 0 || source.y() >= mapHeight) {
		return false;
	}
	if (destination.x() < 0 || destination.x() >= mapWidth || destination.y() < 0 || destination.y() >= mapHeight) {
		return false;
	}
	bool pathOk = source.hasPath(destination);
	
	return pathOk;
}

bool ExplorationManager::canReach(const BaseAgent* pAgent, const BWAPI::TilePosition& destination) {
	return pAgent->getUnit()->hasPath(Position(destination));
}

//TilePosition ExplorationManager::scanForVulnerableBase()
//{
//	TilePosition spot = TilePositions::Invalid;
//	for (int i = 0; i < (int)mSpottedStructures.size(); i++)
//	{
//		if (mSpottedStructures[i]->isActive())
//		{
//			SpottedObject* obj = mSpottedStructures[i];
//			if (obj->getType().isResourceDepot())
//			{
//				if (!isEnemyDetectorCovering(obj->getTilePosition()))
//				{
//					//Broodwar->printf("Found probable vulnerable base at (%d,%d)", obj->getTilePosition().x(), obj->getTilePosition().y());
//					spot = obj->getTilePosition();
//				}
//			}
//		}
//	}
//
//	if (spot.x() < 0)
//	{
//		//Broodwar->printf("Scan: No vulnerable base found");
//	}
//
//	return spot;
//}

bool ExplorationManager::isEnemyDetectorCovering(const BWAPI::TilePosition& position) const {
	return isEnemyDetectorCovering(Position(position));
}


bool ExplorationManager::isEnemyDetectorCovering(const BWAPI::Position& position) const {
	for (int i = 0; i < (int)mSpottedStructures.size(); i++) {
		if (mSpottedStructures[i]->isActive()) {
			shared_ptr<SpottedObject> spottedObject = mSpottedStructures[i];
			if (spottedObject->getType().isDetector()) {
				double distance = spottedObject->getPosition().getDistance(position);
				if (distance <= spottedObject->getType().sightRange()) {
					return true;
				}
			}
		}
	}
	return false;
}

vector<TilePosition> ExplorationManager::findNotCheckedExpansions() const {
	vector<TilePosition> foundPositions;

	for (size_t i = 0; i < mExploreData.size(); ++i) {
		if (mExploreData[i].isExpansion()) {
			double lastVisit = mExploreData[i].secondsSinceLastVisit();

			DEBUG_MESSAGE(utilities::LogLevel_Finest, "ExplorationManager::findNotCheckedExpansions() | " <<
				"Position: " << mExploreData[i].getCenterPosition() << ", LastVisit: " << lastVisit
			);

			if (lastVisit >= config::attack_coordinator::EXPANSION_NOT_CHECKED_TIME) {
				foundPositions.push_back(mExploreData[i].getCenterPosition());
			}
		}
	}

	return foundPositions;
}

void ExplorationManager::removeOccupiedExpansions(std::vector<BWAPI::TilePosition>& expansionPositions) const {
	vector<TilePosition>::iterator expIt = expansionPositions.begin();

	while (expIt != expansionPositions.end()) {
		const TilePosition& expPosition = (*expIt);

		// Check if our own buildings are close
		vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
		bool baseTaken = false;
		for (size_t i = 0; i < agents.size(); ++i) {
			BaseAgent* currentAgent = agents[i];

			if (currentAgent->getUnitType().isResourceDepot() && currentAgent->isAlive()) {
				double distance = expPosition.getDistance(currentAgent->getUnit()->getTilePosition());
				if (distance <= CLOSE_BASE_DISTANCE) {
					baseTaken = true;
					break; // BREAK for agents
				}
			}
		}

		if (baseTaken) {
			expIt = expansionPositions.erase(expIt);
			continue; // CONTINUE for expansion positions
		}

		/// @todo remove teammate player expansions
		

		// Remove enemy bases
		for (size_t i = 0; i < mSpottedStructures.size(); ++i) {
			if (mSpottedStructures[i]->getType().isResourceDepot()) {
				double distance = expPosition.getDistance(mSpottedStructures[i]->getTilePosition());
				if (distance <= CLOSE_BASE_DISTANCE) {
					baseTaken = true;
					break;
				}
			}
		}

		if (baseTaken) {
			expIt = expansionPositions.erase(expIt);
			continue; // CONTINUE for expansion positions
		}

		++expIt;
	}
}