#include "GoliathAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

GoliathAgent::GoliathAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "GoliathAgent";
	//Broodwar->printf("GoliathAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void GoliathAgent::computeActions()
{
	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}
