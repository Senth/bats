#include "ExplorationManager.h"
#include "UnitManager.h"
//#include "VultureAgent.h"
#include "Commander.h"
#include "Squad.h"
#include "BTHAIModule/Source/CoverMap.h"
#include "BTHAIModule/Source/UnitAgent.h"
#include "BTHAIModule/Source/SpottedObject.h"
#include "Config.h"
#include "Helper.h"
#include <set>
#include <cfloat>

using namespace BWAPI;
using namespace BWTA;
using namespace std;
using std::tr1::shared_ptr;
using namespace bats;

ExplorationManager* bats::ExplorationManager::mpsInstance = NULL;

const double CLOSE_BASE_DISTANCE = 12.0;

bats::ExplorationManager::ExplorationManager() {
	mActive = true;
	
	mForceOwn.reset();
	mForceEnemy.reset();
	mCalcTurnCurrent = CalcTurn_First;

	//Add the regions for this map
	for(set<BWTA::Region*>::const_iterator it=getRegions().begin(); it != getRegions().end(); ++it) {
		mExploreData.push_back(ExploreData((*it)->getCenter()));
	}

	//mSiteSetFrame = 0;
	//mExpansionSite = TilePositions::Invalid;

	mFrameLastCall = Broodwar->getFrameCount();
}

bats::ExplorationManager::~ExplorationManager() {
	mpsInstance = NULL;
}

void bats::ExplorationManager::setInactive() {
	mActive = false;
}

bool bats::ExplorationManager::isActive() const {
	return mActive;
}

ExplorationManager* bats::ExplorationManager::getInstance() {
	if (mpsInstance == NULL) {
		mpsInstance = new ExplorationManager();
	}
	return mpsInstance;
}

void bats::ExplorationManager::computeActions() {
	//Don't call too often
	int cFrame = Broodwar->getFrameCount();
	if (cFrame - mFrameLastCall < config::frame_distribution::EXPLORATION_MANAGER) {
		return;
	}
	mFrameLastCall = cFrame;

	if (!mActive) {
		return;
	}
	
	
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


	// Remove dead buildings
	cleanup();


	/// @todo update position on visible buildings that have moved.
}

//TilePosition bats::ExplorationManager::searchExpansionSite()
//{
//	getExpansionSite();
//
//	if (mExpansionSite.x() == -1)
//	{
//		mExpansionSite = CoverMap::getInstance()->findExpansionSite();
//		mSiteSetFrame = Broodwar->getFrameCount();
//		//Broodwar->printf("Found expansion site around (%d,%d)", expansionSite.x(), expansionSite.y());
//	}
//
//	return mExpansionSite;
//}
//
//TilePosition bats::ExplorationManager::getExpansionSite()
//{
//	if (mExpansionSite.x() >= 0)
//	{
//		if (Broodwar->getFrameCount() - mSiteSetFrame > 500)
//		{
//			mExpansionSite = TilePosition(-1, -1);
//		}
//	}
//
//	return mExpansionSite;
//}

//void bats::ExplorationManager::setExpansionSite(TilePosition pos)
//{
//	if (pos.x() >= 0)
//	{
//		mSiteSetFrame = Broodwar->getFrameCount();
//		mExpansionSite = pos;
//	}
//}

TilePosition bats::ExplorationManager::getNextToExplore(const std::tr1::shared_ptr<Squad>& squad) {
	TilePosition currentPos = squad->getCenter();
	TilePosition goal = squad->getGoal();

	//Special case: No goal set, give the squad a new goal directly
	if (goal == TilePositions::Invalid) {
		BWTA::Region* startRegion = getRegion(currentPos); 
		goal = TilePosition(startRegion->getCenter());
		return goal;
	}

	double dist = currentPos.getDistance(goal);

	/// @todo Move accept distance to config
	double acceptDist = 4;
	if (squad->travelsByGround()) {
		acceptDist = 6;
	}

	if (dist <= acceptDist) {
		//Squad is close to goal

		//1. Set region to explored
		setExplored(goal);

		//2. Find new region to explore
		BWTA::Region* startRegion = getRegion(goal);
		BWTA::Region* bestRegion = startRegion;

		if (bestRegion != NULL) {
			int bestLastVisitFrame = getLastVisitFrame(bestRegion);

			if (squad->travelsByGround()) {
				//Ground explorers
				set<BWTA::Region*>::const_iterator regionIt;
				for(regionIt = startRegion->getReachableRegions().begin(); regionIt != startRegion->getReachableRegions().end(); ++regionIt) {
					int cLastVisitFrame = getLastVisitFrame((*regionIt));
					TilePosition c = TilePosition((*regionIt)->getCenter());
					if (cLastVisitFrame <= bestLastVisitFrame) {
						bestLastVisitFrame = cLastVisitFrame;
						bestRegion = (*regionIt);
					}
				}
			} else {
				//Air explorers
				double bestDist = 100000;
				set<BWTA::Region*>::const_iterator regionIt;
				for(regionIt = getRegions().begin(); regionIt != getRegions().end(); ++regionIt) {
					int cLastVisitFrame = getLastVisitFrame((*regionIt));
					TilePosition c = TilePosition((*regionIt)->getCenter());
					double dist = c.getDistance(currentPos);
					if (cLastVisitFrame < bestLastVisitFrame) {
						bestLastVisitFrame = cLastVisitFrame;
						bestRegion = (*regionIt);
						bestDist = dist;
					}
					if (cLastVisitFrame == bestLastVisitFrame && dist < bestDist) {
						bestLastVisitFrame = cLastVisitFrame;
						bestRegion = (*regionIt);
						bestDist = dist;
					}
				}
			}

			TilePosition newGoal = TilePosition(bestRegion->getCenter());
			return newGoal;
			//Broodwar->printf("Explorer: new goal (%d,%d) I am at (%d,%d) agentGoal (%d,%d)", newGoal.x(), newGoal.y(), curPos.x(), curPos.y(), agent->getGoal().x(), agent->getGoal().y());
		}
	}

	return TilePositions::Invalid;
}

void bats::ExplorationManager::setExplored(const BWAPI::TilePosition& goal) {
	bool found = false;
	for (size_t i = 0; i < mExploreData.size(); i++) {
		if (mExploreData[i].matches(goal)) {
			mExploreData[i].lastVisitFrame = Broodwar->getFrameCount();
			found = true;
		}
	}
}

int bats::ExplorationManager::getLastVisitFrame(BWTA::Region* region) {
	for (size_t i = 0; i < mExploreData.size(); i++) {
		if (mExploreData[i].matches(region)) {

			//Check if region is visible. If so, set lastVisitFrame to now
			if (Broodwar->isVisible(mExploreData[i].center)) {
				mExploreData[i].lastVisitFrame = Broodwar->getFrameCount();
			}

			return mExploreData[i].lastVisitFrame;
		}
	}
	
	//Error: No region found
	TilePosition goal = TilePosition(region->getCenter());
	DEBUG_MESSAGE(utilities::LogLevel_Warning, "ExplorationManager::getLastVisitFrame() | " <<
		"Unable to find region for tile (" << goal.x() << ", " << goal.y() << ")!");
	return -1;
}

void bats::ExplorationManager::showIntellData() const {
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

void bats::ExplorationManager::calcOwnForceData() {
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

void bats::ExplorationManager::calcEnemyForceData() {
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

void bats::ExplorationManager::printInfo() const
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

void bats::ExplorationManager::addSpottedUnit(BWAPI::Unit* unit) {
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

void bats::ExplorationManager::unitDestroyed(BWAPI::Unit* unit) {
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

void bats::ExplorationManager::cleanup() {
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

int bats::ExplorationManager::countSpottedBuildingsWithinRange(const BWAPI::TilePosition& position, double range) const {
	int cStructuresInRange = 0;
	for (size_t i = 0; i < mSpottedStructures.size(); i++) {
		if (mSpottedStructures[i]->isActive()) {
			if (position.getDistance(mSpottedStructures[i]->getTilePosition()) <= range) {
				cStructuresInRange++;
			}
		}
	}

	return cStructuresInRange;
}

bool bats::ExplorationManager::hasSpottedBuildingWithinRange(const BWAPI::TilePosition& position, double range) const {
	for (size_t i = 0; i < mSpottedStructures.size(); ++i) {
		if (mSpottedStructures[i]->isActive()) {
			double squaredDistance = getSquaredDistance(mSpottedStructures[i]->getTilePosition(), position);
			if (range * range <= squaredDistance) {
				return true; // QUICK RETURN
			}
		}
	}

	return false;
}

TilePosition bats::ExplorationManager::getClosestSpottedBuilding(const BWAPI::TilePosition& startPosition) const {
	TilePosition pos = BWAPI::TilePositions::Invalid;
	double bestDist = DBL_MAX;

	for (size_t i = 0; i < mSpottedStructures.size(); i++) {
		if (mSpottedStructures[i]->isActive()) {
			double squaredDistance = getSquaredDistance(startPosition, mSpottedStructures[i]->getTilePosition());
			if (squaredDistance < bestDist) {
				bestDist = squaredDistance;
				pos = mSpottedStructures[i]->getTilePosition();
			}
		}
	}

	return pos;
}

vector<shared_ptr<SpottedObject>>& bats::ExplorationManager::getSpottedBuildings() {
	return mSpottedStructures;
}

const vector<shared_ptr<SpottedObject>>& bats::ExplorationManager::getSpottedBuildings() const {
	return mSpottedStructures;
}

bool bats::ExplorationManager::hasSpottedBuilding() const {
	if (mSpottedStructures.size() > 0) {
		return true;
	}
	return false;
}

bool bats::ExplorationManager::canReach(BWAPI::TilePosition source, BWAPI::TilePosition destination) {
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

bool bats::ExplorationManager::canReach(UnitAgent* pAgent, BWAPI::TilePosition destination) {
	return pAgent->getUnit()->hasPath(Position(destination));
}

//TilePosition bats::ExplorationManager::scanForVulnerableBase()
//{
//	TilePosition spot = TilePosition(-1, -1);
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

bool bats::ExplorationManager::isEnemyDetectorCovering(const BWAPI::TilePosition& position) const {
	return isEnemyDetectorCovering(Position(position));
}


bool bats::ExplorationManager::isEnemyDetectorCovering(const BWAPI::Position& position) const {
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

vector<TilePosition> bats::ExplorationManager::findNotCheckedExpansions() const {
	vector<TilePosition> foundPositions;

	// Iterate through all expansions sites
	const set<BWTA::BaseLocation*>& baseLocations = BWTA::getBaseLocations();
	set<BWTA::BaseLocation*>::const_iterator baseLocationIt;
	for (baseLocationIt = baseLocations.begin(); baseLocationIt != baseLocations.end(); ++baseLocationIt) {
		TilePosition basePosition = (*baseLocationIt)->getTilePosition();

		// Search all regions for the base position
		ExploreData foundRegion(TilePositions::Invalid);
		vector<ExploreData>::const_iterator exploreRegionIt = mExploreData.begin();
		while (foundRegion.center == TilePositions::Invalid && exploreRegionIt != mExploreData.end()) {
			if (exploreRegionIt->isWithin(basePosition)) {
				foundRegion = *exploreRegionIt;
			}
			++exploreRegionIt;
		}

		DEBUG_MESSAGE_CONDITION(foundRegion.center == TilePositions::Invalid,
			utilities::LogLevel_Severe,
			"ExplorationManager::findNotCheckdExpansions() | Did not find any region for " <<
			"expansion located at (" << basePosition.x() << ", " << basePosition.y() << ")!");

		// Add the region if it hasn't been visited for a "long" time
		if (foundRegion.center != TilePositions::Invalid && 
			foundRegion.secondsSinceLastVisit() >= config::attack_coordinator::EXPANSION_NOT_CHECKED_TIME)
		{
			foundPositions.push_back(basePosition);	
		}
	}

	return foundPositions;
}

void bats::ExplorationManager::removeOccupiedExpansions(std::vector<BWAPI::TilePosition>& expansionPositions) const {
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