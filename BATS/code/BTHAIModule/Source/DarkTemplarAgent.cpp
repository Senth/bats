#include "DarkTemplarAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "ExplorationManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

DarkTemplarAgent::DarkTemplarAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "DarkTemplarAgent";
	//Broodwar->printf("DarkTemplarAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void DarkTemplarAgent::computeActions()
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
