#include "HydraliskAgent.h"
#include "PFManager.h"
#include "Commander.h"
#include "TargetingAgent.h"

HydraliskAgent::HydraliskAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "HydraliskAgent";
	//Broodwar->printf("HydraliskAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void HydraliskAgent::computeActions()
{
	if (Broodwar->self()->hasResearched(TechTypes::Lurker_Aspect))
	{
		Squad* sq = Commander::getInstance()->getSquad(squadID);
		if (sq != NULL)
		{
			if (sq->morphsTo().getID() == UnitTypes::Zerg_Lurker.getID())
			{
				if (noUnitsInWeaponRange() == 0)
				{
					if (Broodwar->canMake(unit, UnitTypes::Zerg_Lurker))
				{
						unit->morph(UnitTypes::Zerg_Lurker);
						return;
					}
				}
			}
		}
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
