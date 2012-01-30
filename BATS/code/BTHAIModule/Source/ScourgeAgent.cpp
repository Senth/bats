#include "ScourgeAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

ScourgeAgent::ScourgeAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "ScourgeAgent";
	//Broodwar->printf("ScourgeAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void ScourgeAgent::computeActions()
{
	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}
