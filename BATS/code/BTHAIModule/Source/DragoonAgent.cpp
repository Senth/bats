#include "DragoonAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "Commander.h"
#include "TargetingAgent.h"
#include "Profiler.h"

DragoonAgent::DragoonAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "DragoonAgent";
	//Broodwar->printf("DragoonAgent created (%s)", unit->getType().getName().c_str());
	goal = TilePosition(-1, -1);
}

void DragoonAgent::computeActions()
{
	Squad* sq = Commander::getInstance()->getSquad(squadID);
	if (sq != NULL)
	{
		if (sq->isKite())
		{
			computeKitingActions();
			return;
		}
	}
	
	if (chargeShields())
	{
		return;
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	bool defensive = false;
	if (unit->getGroundWeaponCooldown() > 0)
	{
		defensive = true;
	}
	PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
}
