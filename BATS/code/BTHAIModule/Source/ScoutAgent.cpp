#include "ScoutAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

ScoutAgent::ScoutAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "ScoutAgent";
	//Broodwar->printf("ScoutAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void ScoutAgent::computeActions()
{
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

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}
