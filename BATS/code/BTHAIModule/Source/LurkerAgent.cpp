#include "LurkerAgent.h"
#include "PFManager.h"
#include "Commander.h"
#include "ExplorationManager.h"
#include "TargetingAgent.h"

LurkerAgent::LurkerAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "LurkerAgent";
	//Broodwar->printf("LurkerAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void LurkerAgent::computeActions()
{
	int eCnt = enemyGroundUnitsWithinRange(unit->getType().groundWeapon().maxRange());
	eCnt += enemyAirToGroundUnitsWithinRange(unit->getType().groundWeapon().maxRange());
	if (ExplorationManager::enemyIsTerran() && eCnt == 0)
	{
		//Check if we have any of those nasty tanks around
		eCnt += enemySiegedTanksWithinRange(unit->getTilePosition());
	}
	
	if (eCnt > 0 && !unit->isBurrowed())
	{
		if (unit->burrow()) return;
	}
	if (eCnt == 0 && unit->isBurrowed())
	{
		unit->unburrow();
		return;
	}

	Squad* sq = Commander::getInstance()->getSquad(squadID);
	if (sq != NULL)
	{
		if (sq->isKite())
		{
			computeKitingActions();
			return;
		}
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}
	
	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}
