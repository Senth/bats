#include "WraithAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

WraithAgent::WraithAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "WraithAgent";
}

void WraithAgent::computeActions()
{
	if (checkUseCloak())
	{
		return;
	}

	/// @todo kite
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

bool WraithAgent::checkUseCloak()
{
	TechType cloak = TechTypes::Cloaking_Field;
	if (Broodwar->self()->hasResearched(cloak))
	{
		if (!unit->isCloaked())
		{
			if (unit->getEnergy() >= 25 && !isDetectorWithinRange(192))
			{
				int range = 10 * 32;
				int eCnt = enemyUnitsWithinRange(range);
				if (eCnt > 0)
				{
					unit->useTech(cloak);
					return true;
				}
			}
		}
		//Dont decloak since it is costly to first use cloak and
		//keeping it up is cheap.
		/*if (unit->isCloaked())
		{
			int range = 10 * 32;
			int eCnt = enemyUnitsWithinRange(range);
			if (eCnt == 0)
			{
				unit->decloak();
				return true;
			}
		}*/
	}
	return false;
}
