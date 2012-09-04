#include "UnitAgent.h"
#include "WorkerAgent.h"
#include "AgentManager.h"
#include "PFManager.h"
#include "CoverMap.h"
#include "BatsModule/include/BuildPlanner.h"
#include "BatsModule/include/SquadManager.h"
#include "BatsModule/include/SquadDefs.h"
#include "BatsModule/include/Squad.h"
#include "BatsModule/include/Config.h"
#include "BatsModule/include/DefenseManager.h"
#include "BatsModule/include/Helper.h"
#include "BatsModule/include/UnitHelper.h"
#include "ResourceManager.h"
#include "PathFinder.h"
#include "Profiler.h"
#include <sstream>
#include <iomanip>

using namespace BWAPI;
using namespace std;

map<const BWAPI::Unit*, int> WorkerAgent::msRepairUnits; 
CoverMap* WorkerAgent::msCoverMap = NULL;

WorkerAgent::WorkerAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	setState(GATHER_MINERALS);
	agentType = "WorkerAgent";
	mRepairUnit = NULL;

	if (NULL == msCoverMap) {
		msCoverMap = CoverMap::getInstance();
	}
}

void WorkerAgent::destroyed() {
	if (mCurrentState == MOVE_TO_SPOT || mCurrentState == FIND_BUILDSPOT) {
		if (!bats::BuildPlanner::isZerg()) {
			//Broodwar->printf("Worker building %s destroyed", toBuild.getName().c_str());
			bats::BuildPlanner::getInstance()->handleWorkerDestroyed(mToBuild, unitID);
			msCoverMap->clearTemp(mToBuild, mBuildSpot);
			setState(GATHER_MINERALS);
		}
	}
	// For Terran, send another SCV to continue building if we have started to build
	else if (mCurrentState == CONSTRUCT && Broodwar->self()->getRace() == Races::Terran) {
		const Unit* targetBuilding = getUnit()->getOrderTarget();
		if (NULL != targetBuilding) {
			bats::BuildPlanner::getInstance()->findAnotherBuilder(targetBuilding);
		}
	}
	// For Terran, remove repair scv from the unit it is repairing
	else if (mCurrentState == REPAIRING && Broodwar->self()->getRace() == Races::Terran) {
		const Unit* repairUnit = getUnit()->getOrderTarget();
		if (NULL != repairUnit) {
			decreaseRepairScvs(repairUnit);
		}
	}
}

Unit* WorkerAgent::getEnemyUnit()
{
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			if (!(*i)->getType().isBuilding())
			{
				double dist = unit->getTilePosition().getDistance((*i)->getTilePosition());
				if (dist <= 3)
				{
					return (*i);
				}
			}
		}	
	}
	return NULL;
}

Unit* WorkerAgent::getEnemyBuilding()
{
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			if ((*i)->getType().isBuilding())
			{
				double dist = unit->getTilePosition().getDistance((*i)->getTilePosition());
				if (dist <= 5)
				{
					return (*i);
				}
			}
		}	
	}
	return NULL;
}

Unit* WorkerAgent::getEnemyWorker()
{
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			if ((*i)->getType().isWorker())
			{
				double dist = unit->getTilePosition().getDistance((*i)->getTilePosition());
				if (dist <= 5)
				{
					return (*i);
				}
			}
		}	
	}
	return NULL;
}

void WorkerAgent::handleKitingWorker()
{
	//Bring them back to base
	unit->rightClick(Position(Broodwar->self()->getStartLocation()));
}

void WorkerAgent::printGraphicDebugInfo() const
{
	if (bats::config::debug::GRAPHICS_VERBOSITY == bats::config::debug::GraphicsVerbosity_Off) {
		return;
	}

	if (!isAlive()) return;
	if (!unit->isCompleted()) return;

	BaseAgent::printGraphicDebugSelectedUnits();

	if (bats::config::debug::modules::AGENT_WORKER) {
		const Position& unitPos = unit->getPosition();
		Position goalPos = Position(goal);

		// Low
		// Draw boxes when building stuff
		if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Low) {
			if (mCurrentState == MOVE_TO_SPOT || mCurrentState == CONSTRUCT)
			{
				if (mBuildSpot.x() > 0)
				{
					int w = mToBuild.tileWidth() * TILE_SIZE;
					int h = mToBuild.tileHeight() * TILE_SIZE;

					Position b = Position(mBuildSpot.x()*TILE_SIZE + w/2, mBuildSpot.y()*TILE_SIZE + h/2);
					Broodwar->drawLineMap(unitPos.x(), unitPos.y(), b.x(), b.y(), Colors::Teal);
					Broodwar->drawBoxMap(mBuildSpot.x()*TILE_SIZE, mBuildSpot.y()*TILE_SIZE, mBuildSpot.x()*TILE_SIZE+w, mBuildSpot.y()*TILE_SIZE+h, Colors::Blue);
				}
			}
		}


		// Medium
		// Draw line to mineral/gas unit is mining from
		if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Medium) {
			if (mCurrentState == GATHER_MINERALS || mCurrentState == GATHER_GAS)
			{
				Unit* target = unit->getTarget();
				if (target != NULL)
				{
					const Position& targetPos = target->getPosition();
					Broodwar->drawLineMap(unitPos.x(), unitPos.y(), targetPos.x(), targetPos.y(), Colors::Teal);
				}
			}
			
			else if (mCurrentState == REPAIRING)
			{
				Unit* target = unit->getOrderTarget();
				if (target != NULL)
				{
					Position b = Position(target->getPosition());
					Broodwar->drawLineMap(unitPos.x(), unitPos.y(), b.x() ,b.y(), Colors::Green);
					Broodwar->drawTextMap(unitPos.x(), unitPos.y(), "Repairing %s", target->getType().getName().c_str());
				}
			}
			
			else if (mCurrentState == CONSTRUCT)
			{
				Unit* target = unit->getOrderTarget();
				if (target != NULL)
				{
					Position b = target->getPosition();
					Broodwar->drawTextMap(unitPos.x(), unitPos.y(), "Constructing %s", target->getType().getName().c_str());
				}
			}

			// Does something arbitrary, draw goal
			else if (mCurrentState != MOVE_TO_SPOT && mCurrentState != FIND_BUILDSPOT) {
				Broodwar->drawLineMap(unitPos.x(), unitPos.y(), goalPos.x(), goalPos.y(), Colors::White);
			}
		}
	}
}

string WorkerAgent::getDebugString() const {
	stringstream ss;
	
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Medium) {
		string state = getStateAsText() + ": ";
		ss << setw(bats::config::debug::GRAPHICS_COLUMN_WIDTH) << state << goal << "\n";
	}

	return BaseAgent::getDebugString() + ss.str();
}

void WorkerAgent::computeActions()
{
	// @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
	if (getSquadId().isValid())
	{
		//Worker is in a squad

		bats::SquadPtr squad = msSquadManager->getSquad(getSquadId());
		if (NULL != squad) {

			// Terran
			if (Broodwar->self()->getRace() == Races::Terran) {
				if (NULL == mRepairUnit) {
					// Find unit to repair
					mRepairUnit = findRepairUnit();
					if (NULL != mRepairUnit) {
						getUnit()->repair(const_cast<Unit*>(mRepairUnit));
						increaseRepairScvs(mRepairUnit);
					}
					// No unit to repair, just follow
					else {
						computeMoveAction();
					}
				}
				// Already repairing, check when repair is done
				else {
					testRepairDone();
				}
			} else {
				computeMoveAction();
			}

			
		} else {
			ERROR_MESSAGE(false, "Squad is null for WorkerAgent, why is it this?");
		}
	} else {
		// Check if under attack, report to Defense Manager
		if (unit->isUnderAttack()) {
			msDefenseManager->onWorkerUnderAttack(this);
		}

		//Check if workers are too far away from a base when attacking
		if (mCurrentState == ATTACKING)
		{
			if (unit->getTarget() != NULL)
			{
				BaseAgent* base = AgentManager::getInstance()->getClosestBase(unit->getTilePosition());
				if (base != NULL)
				{
					double dist = base->getUnit()->getTilePosition().getDistance(unit->getTilePosition());
					if (dist > 25)
					{
						//Stop attacking. Return home
						unit->stop();
						unit->rightClick(base->getUnit());
						setState(GATHER_MINERALS);
						return;
					}
				}
			}
			else
			{
				//No target, return to gather minerals
				setState(GATHER_MINERALS);
				return;
			}
		}

		if (mCurrentState == GATHER_GAS)
		{
			if (unit->isIdle())
			{
				//Not gathering gas. Reset.
				setState(GATHER_MINERALS);
			}
		}
	
		if (mCurrentState == REPAIRING)
		{
			if (testRepairDone()) {
				setState(GATHER_MINERALS);
				BaseAgent* base = AgentManager::getInstance()->getClosestBase(unit->getTilePosition());
				if (base != NULL)
				{
					unit->rightClick(base->getUnit());
					return;
				}
			}
		}

		if (mCurrentState == GATHER_MINERALS)
		{
			// Find units to repair
			const Unit* unitToRepair = findRepairUnit();
			if (NULL != unitToRepair) {
				assignToRepair(unitToRepair);
			}
			// If idle -> find a mineral to mine from
			else if (unit->isIdle() || !unit->isGatheringMinerals()) {
				Unit* mineral = msCoverMap->findClosestMineral(unit->getTilePosition());
				if (mineral != NULL) {
					unit->rightClick(mineral);
				}
			}


		}

		if (mCurrentState == FIND_BUILDSPOT)
		{
			if (mBuildSpot == TilePositions::Invalid) {
				mBuildSpot = msCoverMap->findBuildSpot(mToBuild, getUnitID());
			}
			if (mBuildSpot != TilePositions::Invalid)
			{
				msCoverMap->fillTemp(mToBuild, mBuildSpot);
				setState(MOVE_TO_SPOT);
			}
		}

		if (mCurrentState == MOVE_TO_SPOT)
		{
			if (!buildSpotExplored())
			{
				unit->rightClick(Position(mBuildSpot));
			}

			if (buildSpotExplored() && !unit->isConstructing())
			{
				bool startedBuilding = unit->build(mBuildSpot, mToBuild);
				
				// Can't build at selected spot, get a new one.
				if (!startedBuilding) {
					setState(FIND_BUILDSPOT);
					msCoverMap->clearTemp(mToBuild, mBuildSpot);
					mBuildSpot = TilePositions::Invalid;
				}
			}

			if (unit->isConstructing())
			{
				setState(CONSTRUCT);
			}
		}

		if (mCurrentState == CONSTRUCT)
		{
			if (hasCompletedBuilding())
			{
				//Build finished.
				BaseAgent* agent = AgentManager::getInstance()->getClosestBase(unit->getTilePosition());
				if (agent != NULL)
				{
					unit->rightClick(agent->getUnit()->getPosition());
				}
				setState(GATHER_MINERALS);
			}
			else if (!getUnit()->isConstructing()) {
				setState(FIND_BUILDSPOT);
				msCoverMap->clearTemp(mToBuild, mBuildSpot);
				mBuildSpot = TilePositions::Invalid;
			}
		}
	}
}

bool WorkerAgent::hasCompletedBuilding() const
{
	/// @author Matteus Magnusson (matteus.magnusson@gmail.com)
	/// Changed logic to test if the building is actually done, not begun
	const set<Unit*> tileUnits = Broodwar->getUnitsOnTile(mBuildSpot.x(), mBuildSpot.y());
	set<Unit*>::const_iterator tileUnitIt;
	for (tileUnitIt = tileUnits.begin(); tileUnitIt != tileUnits.end(); ++tileUnitIt)
	{
		// Only our own units that exist
		if ((*tileUnitIt)->exists() && (*tileUnitIt)->getPlayer() == Broodwar->self())
		{
			// The building exist and is completed
			if ((*tileUnitIt)->getType() == mToBuild && (*tileUnitIt)->isCompleted())
			{
				return true;
			}
		}
	}
	return false;
}

bool WorkerAgent::areaFree() const {
	if (mToBuild.isRefinery()) {
		return true;
	}

	const vector<Unit*>& teamUnits = bats::UnitHelper::getTeamUnits();
	TilePosition maxPos = mBuildSpot;
	maxPos.x() += mToBuild.tileWidth() - 1;
	maxPos.y() += mToBuild.tileHeight() - 1;
	if (bats::UnitHelper::unitsInArea(teamUnits, mBuildSpot, maxPos, getUnitID())) {
		return false;
	}
	
	return true;
}

bool WorkerAgent::buildSpotExplored() const
{
	// Check if it's visible.
	return Broodwar->isVisible(mBuildSpot);

	//int sightDist = 64;
	//if (toBuild.isRefinery())
	//{
	//	sightDist = 160; //5 tiles
	//}

	//double dist = unit->getPosition().getDistance(Position(buildSpot));
	////Broodwar->printf("Dist=%d, toReach=%d", (int)dist, sightDist);
	//if (dist > sightDist)
	//{
	//	//Broodwar->printf("Not there");
	//	return false;
	//}
	////Broodwar->printf("Arrived");
	//return true;
}

WorkerAgent::States WorkerAgent::getState() const
{
	return mCurrentState;
}

void WorkerAgent::setState(States state)
{
	mCurrentState = state;
	
	if (state == GATHER_MINERALS)
	{
		mBuildSpot = TilePositions::Invalid;
	}
}

bool WorkerAgent::assignToRepair(const Unit* repairUnit)
{
	if (NULL == repairUnit) {
		return false;
	}

	if (unit->isIdle() || (unit->isGatheringMinerals() && !unit->isCarryingMinerals()))
	{
		setState(REPAIRING);
		mRepairUnit = repairUnit;
		unit->repair(const_cast<Unit*>(repairUnit));
		increaseRepairScvs(repairUnit);
		return true;
	} else {
		return false;
	}
}

bool WorkerAgent::assignToFinishBuild(const Unit* building)
{
	if (isFreeWorker()) {
		setState(REPAIRING);
		unit->rightClick(const_cast<Unit*>(building));
		return true;
	}
	return false;
}

bool WorkerAgent::canBuild(const UnitType& type) const
{
	//Make sure we have some spare resources so we dont drain
	//required minerals for our units.
	//if (Broodwar->self()->minerals() < type.mineralPrice() + 100)
	if (Broodwar->self()->minerals() < type.mineralPrice())
	{
		return false;
	}

	if (unit->isIdle() || (unit->isGatheringMinerals() && !unit->isCarryingMinerals()))
	{
		if (Broodwar->canMake(unit, type))
		{
			return true;
		}
	}
	return false;
}

bool WorkerAgent::assignToBuild(const UnitType& type)
{
	mToBuild = type;
	mBuildSpot = msCoverMap->findBuildSpot(mToBuild, getUnitID());
	if (mBuildSpot != TilePositions::Invalid)
	{
		msCoverMap->fillTemp(mToBuild, mBuildSpot);
		ResourceManager::getInstance()->lockResources(mToBuild);
		setState(FIND_BUILDSPOT);
		return true;
	}
	else
	{
		//Broodwar->printf("No buildspot found for %s", type.getName().c_str());
		return false;
	}
}

void WorkerAgent::reset()
{
	// Reset build spot
	if (mToBuild != UnitTypes::None) {
		if (mBuildSpot != TilePositions::Invalid) {
			msCoverMap->clearTemp(mToBuild, mBuildSpot);
			mBuildSpot = TilePositions::Invalid;
		}
		ResourceManager::getInstance()->unlockResources(mToBuild);
		mToBuild = UnitTypes::None;
		
	}

	if (unit->isConstructing())
	{
		unit->cancelConstruction();
	}

	setState(GATHER_MINERALS);
	unit->stop();
	BaseAgent* base = AgentManager::getInstance()->getClosestBase(unit->getTilePosition());
	if (base != NULL)
	{
		unit->rightClick(base->getUnit()->getPosition());
	}
}

bool WorkerAgent::isConstructing(const UnitType& type) const
{
	if (mCurrentState == FIND_BUILDSPOT || mCurrentState == MOVE_TO_SPOT || mCurrentState == CONSTRUCT)
	{
		if (type == UnitTypes::None)
		{
			return true;
		}
		else if (type == mToBuild)
		{
			return true;
		}
	}
	return false;
}

string WorkerAgent::getStateAsText() const
{
	string strReturn;
	switch(mCurrentState)
	{
	case GATHER_MINERALS:
		strReturn = "Minerals";
		break;
	case GATHER_GAS:
		strReturn = "Gas";
		break;
	case FIND_BUILDSPOT:
		strReturn = "FindBuild";
		break;
	case MOVE_TO_SPOT:
		strReturn = "MoveToSpot";
		break;
	case CONSTRUCT:
		strReturn = "Construct";
		break;
	case REPAIRING:
		strReturn = "Repair";
		break;
	};
	return strReturn;
}

const BWAPI::Unit* WorkerAgent::findRepairUnit() const {
	// No minerals / gas, can't repair
	if (Broodwar->self()->minerals() == 0 || Broodwar->self()->gas() == 0) {
		return NULL;
	}

	const Unit* bestUnit = NULL;
	double bestFractionHealth = 0.0;

	bool inSquad = getSquadId().isValid();

	// Is worker in a squad or not?
	std::vector<const BaseAgent*> agents;
	if (inSquad) {
		bats::SquadCstPtr squad = msSquadManager->getSquad(getSquadId());
		if (NULL != squad) {
			const vector<const UnitAgent*>& squadUnits = squad->getUnits();
			agents = *reinterpret_cast<const vector<const BaseAgent*>*>(&squadUnits);
		}
	} else {
		agents = const_cast<const AgentManager*>(AgentManager::getInstance())->getAgents();
	}

	for (size_t i = 0; i < agents.size(); ++i) {
		if ((agents[i]->getUnitType().isBuilding() ||
			agents[i]->getUnitType().isMechanical()) &&
			agents[i]->isCompleted() &&
			canMoreScvsRepairThisUnit(agents[i]->getUnit()))
		{

			int healthWhenToRepair = agents[i]->getUnitType().maxHitPoints() - bats::config::unit::scv::REPAIR_UNIT_HEALTH_LOST;
			if (agents[i]->getUnit()->getHitPoints() <= healthWhenToRepair) {
				// Only test range if not in a squad
				if (inSquad || bats::isWithinRange(agents[i]->getUnit()->getTilePosition(), getUnit()->getTilePosition(), bats::config::unit::scv::REPAIR_SEARCH_DISTANCE)) {
					// Calculate fraction of health
					double healthFractionLeft = static_cast<double>(agents[i]->getUnit()->getHitPoints()) / agents[i]->getUnitType().maxHitPoints();
					if (healthFractionLeft > bestFractionHealth) {
						bestUnit = agents[i]->getUnit();
						bestFractionHealth = healthFractionLeft;
					}
				}
			}
		}
	}

	return bestUnit;
}

bool WorkerAgent::canMoreScvsRepairThisUnit(const BWAPI::Unit* unit) {
	map<const BWAPI::Unit*, int>::const_iterator repairIt = msRepairUnits.find(unit);

	// True if not found or less than max
	return repairIt == msRepairUnits.end() || repairIt->second < bats::config::unit::scv::REPAIRERS_PER_UNIT_MAX;
}

void WorkerAgent::decreaseRepairScvs(const BWAPI::Unit* unit) {
	map<const BWAPI::Unit*, int>::iterator repairIt = msRepairUnits.find(unit);

	if (repairIt != msRepairUnits.end()) {
		repairIt->second--;

		// Remove if no repairers
		if (repairIt->second == 0) {
			msRepairUnits.erase(repairIt);
		}
	}
}

void WorkerAgent::increaseRepairScvs(const BWAPI::Unit* unit) {
	msRepairUnits[unit]++;
}

bool WorkerAgent::testRepairDone() {
	if (NULL == mRepairUnit) {
		return true;
	}

	bool repairDone = false;

	// Repair unit died
	if (!mRepairUnit->exists()) {
		repairDone = true;
	}

	// Full health, done
	else if (mRepairUnit->getHitPoints() >= mRepairUnit->getType().maxHitPoints()) {
		repairDone = true;
	}

	// No minerals or gas
	else if (Broodwar->self()->minerals() == 0 || Broodwar->self()->gas() == 0) {
		repairDone = true;
	}

	// Worker idle
	else if (getUnit()->isIdle()) {
		repairDone = true;
	}

	if (repairDone) {
		decreaseRepairScvs(mRepairUnit);
		mRepairUnit = NULL;
	}

	return NULL == mRepairUnit;
}