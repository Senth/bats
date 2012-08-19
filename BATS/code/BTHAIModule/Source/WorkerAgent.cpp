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
#include "ResourceManager.h"
#include "PathFinder.h"
#include "Profiler.h"
#include <sstream>

using namespace BWAPI;
using namespace std;

CoverMap* WorkerAgent::msCoverMap = NULL;

WorkerAgent::WorkerAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	setState(GATHER_MINERALS);
	agentType = "WorkerAgent";

	if (NULL == msCoverMap) {
		msCoverMap = CoverMap::getInstance();
	}
}

void WorkerAgent::destroyed()
{
	if (mCurrentState == MOVE_TO_SPOT || mCurrentState == CONSTRUCT || mCurrentState == FIND_BUILDSPOT)
	{
		if (!bats::BuildPlanner::isZerg())
		{
			//Broodwar->printf("Worker building %s destroyed", toBuild.getName().c_str());
			bats::BuildPlanner::getInstance()->handleWorkerDestroyed(mToBuild, unitID);
			msCoverMap->clearTemp(mToBuild, mBuildSpot);
			setState(GATHER_MINERALS);
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
	if (bats::config::debug::GRAPHICS_VERBOSITY == bats::config::debug::GraphicsVerbosity_Off ||
		bats::config::debug::modules::AGENT_WORKER == false)
	{
		return;
	}

	if (!isAlive()) return;
	if (!unit->isCompleted()) return;



	const Position& unitPos = unit->getPosition();
	Position goalPos = Position(goal);

	// Low
	// Draw boxes when building stuff
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Low) {
		if (mCurrentState == MOVE_TO_SPOT || mCurrentState == CONSTRUCT)
		{
			if (mBuildSpot.x() > 0)
			{
				int w = mToBuild.tileWidth() * 32;
				int h = mToBuild.tileHeight() * 32;

				Position b = Position(mBuildSpot.x()*32 + w/2, mBuildSpot.y()*32 + h/2);
				Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Teal);

				Broodwar->drawBoxMap(mBuildSpot.x()*32,mBuildSpot.y()*32,mBuildSpot.x()*32+w,mBuildSpot.y()*32+h,Colors::Blue,false);
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
				Broodwar->drawLineMap(unitPos.x(),unitPos.y(),targetPos.x(),targetPos.y(),Colors::Teal);
			}
		}

		if (unit->isRepairing())
		{
			Unit* target = unit->getOrderTarget();
			if (target != NULL)
			{
				Position a = Position(unit->getPosition());
				Position b = Position(target->getPosition());
				Broodwar->drawLine(CoordinateType::Map,a.x(),a.y(),b.x(),b.y(),Colors::Green);

				Broodwar->drawText(CoordinateType::Map, unit->getPosition().x(), unit->getPosition().y(), "Repairing %s", target->getType().getName().c_str());
			}
		}
		else if (unit->isConstructing())
		{
			Unit* target = unit->getOrderTarget();
			if (target != NULL)
			{
				Position b = target->getPosition();
				Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Green);

				Broodwar->drawTextMap(unitPos.x(), unitPos.y(), "Constructing %s", target->getType().getName().c_str());
			}
		}
	}
}

void WorkerAgent::computeActions()
{
	// @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
	if (getSquadId() != bats::SquadId::INVALID_KEY)
	{
		//Worker is in a squad

		bats::SquadPtr squad = msSquadManager->getSquad(getSquadId());
		if (NULL != squad) {
			computeMoveAction();

			/// @todo repair something?
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
			if (!unit->isRepairing())
			{
				setState(GATHER_MINERALS);
				BaseAgent* base = AgentManager::getInstance()->getClosestBase(unit->getTilePosition());
				if (base != NULL)
				{
					unit->rightClick(base->getUnit());
					return;
				}
			}
			else
			{
				return;
			}
		}

		if (mCurrentState == GATHER_MINERALS)
		{
			if (unit->isIdle())
			{
				Unit* mineral = msCoverMap->findClosestMineral(unit->getTilePosition());
				if (mineral != NULL)
				{
					unit->rightClick(mineral);
				}
			}
		}

		if (mCurrentState == FIND_BUILDSPOT)
		{
			msCoverMap->clearTemp(mToBuild, mBuildSpot);
			mBuildSpot = msCoverMap->findBuildSpot(mToBuild);
			if (mBuildSpot != TilePositions::Invalid)
			{
				setState(MOVE_TO_SPOT);
			}
		}

		if (mCurrentState == MOVE_TO_SPOT)
		{
			msCoverMap->fillTemp(mToBuild, mBuildSpot);
			if (!buildSpotExplored())
			{
				unit->rightClick(Position(mBuildSpot));
			}

			if (buildSpotExplored() && !unit->isConstructing())
			{
				if (areaFree())
				{
					bool ok = unit->build(mBuildSpot, mToBuild);
					if (!ok)
					{
						msCoverMap->blockPosition(mBuildSpot);
						//Cant build at selected spot, get a new one.
						setState(FIND_BUILDSPOT);
					}
				}
				else
				{
					//Cant build at selected spot, get a new one.
					setState(FIND_BUILDSPOT);
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

bool WorkerAgent::areaFree() const
{
	if (mToBuild.isRefinery())
	{
		return true;
	}

	if (AgentManager::getInstance()->unitsInArea(mBuildSpot, mToBuild.tileWidth(), mToBuild.tileHeight(), unit->getID()))
	{
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

bool WorkerAgent::assignToRepair(Unit* building)
{
	if (unit->isIdle() || (unit->isGatheringMinerals() && !unit->isCarryingMinerals()))
	{
		setState(REPAIRING);
		unit->repair(building);
		return true;
	}
	return false;
}

bool WorkerAgent::assignToFinishBuild(Unit* building)
{
	if (unit->isIdle() || (unit->isGatheringMinerals() && !unit->isCarryingMinerals()))
	{
		setState(REPAIRING);
		unit->rightClick(building);
		return true;
	}
	return false;
}

bool WorkerAgent::canBuild(UnitType type) const
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

bool WorkerAgent::assignToBuild(UnitType type)
{
	mToBuild = type;
	mBuildSpot = msCoverMap->findBuildSpot(mToBuild);
	if (mBuildSpot != TilePositions::Invalid)
	{
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
	msCoverMap->clearTemp(mToBuild, mBuildSpot);

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

bool WorkerAgent::isConstructing(UnitType type) const
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

/** Returns the state of the agent as text. Good for printouts. */
string WorkerAgent::getStateAsText() const
{
	string strReturn;
	switch(mCurrentState)
	{
	case GATHER_MINERALS:
		strReturn = "GATHER_MINERALS";
		break;
	case GATHER_GAS:
		strReturn = "GATHER_GAS";
		break;
	case FIND_BUILDSPOT:
		strReturn = "FIND_BUILDSPOT";
		break;
	case MOVE_TO_SPOT:
		strReturn = "MOVE_TO_SPOT";
		break;
	case CONSTRUCT:
		strReturn = "CONSTRUCT";
		break;
	case REPAIRING:
		strReturn = "REPAIRING";
		break;
	};
	return strReturn;
}
