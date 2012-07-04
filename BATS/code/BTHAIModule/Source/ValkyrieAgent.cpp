#include "ValkyrieAgent.h"
#include "PFManager.h"
#include "Commander.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

ValkyrieAgent::ValkyrieAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "ValkyrieAgent";
}

void ValkyrieAgent::computeActions()
{
	/// @todo kiting
	//Squad* sq = Commander::getInstance()->getSquad(squadID);
	//if (sq != NULL)
	//{
	//	if (sq->isKite())
	//	{
	//		computeKitingActions();
	//		return;
	//	}
	//}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	if (useDefensiveMode())
	{
		computeAttackingActions(true);
	}
	else
	{
		computeAttackingActions();
	}
}
