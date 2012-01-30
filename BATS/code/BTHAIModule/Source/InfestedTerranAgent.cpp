#include "InfestedTerranAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

InfestedTerranAgent::InfestedTerranAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "InfestedTerranAgent";
	//Broodwar->printf("InfestedTerranAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void InfestedTerranAgent::computeActions()
{
	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}
