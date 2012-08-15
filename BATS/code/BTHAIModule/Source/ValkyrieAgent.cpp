#include "ValkyrieAgent.h"
#include "PFManager.h"
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


	if (useDefensiveMode())
	{
		computeMoveAction(true);
	}
	else
	{
		bool attackingEnemy = findAndTryAttack();
		if (!attackingEnemy) {
			computeMoveAction();
		}
	}
}
