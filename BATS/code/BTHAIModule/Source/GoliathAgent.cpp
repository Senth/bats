#include "GoliathAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

GoliathAgent::GoliathAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "GoliathAgent";
}

void GoliathAgent::computeActions()
{
	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	computeAttackingActions();
}
