#include "ZealotAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "Commander.h"
#include "Squad.h"
#include "RushSquad.h"
#include "TargetingAgent.h"
#include "Profiler.h"

using namespace BWAPI;
using namespace std;

ZealotAgent::ZealotAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "ZealotAgent";
	//Broodwar->printf("ZealotAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void ZealotAgent::computeActions()
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
