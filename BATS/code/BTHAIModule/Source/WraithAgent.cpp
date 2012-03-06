#include "WraithAgent.h"
#include "PFManager.h"
#include "Commander.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

WraithAgent::WraithAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "WraithAgent";
	//Broodwar->printf("WraithAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void WraithAgent::computeActions()
{
	if (checkUseCloak())
	{
		return;
	}

	Squad* sq = Commander::getInstance()->getSquad(squadID);
	if (sq != NULL)
	{
		if (sq->isKite())
		{
			computeKitingActions();
			return;
		}
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	bool defensive = useDefensiveMode();
	PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
}

bool WraithAgent::checkUseCloak()
{
	TechType cloak = TechTypes::Cloaking_Field;
	if (Broodwar->self()->hasResearched(cloak))
	{
		if (!unit->isCloaked())
		{
			if (unit->getEnergy() >= 25 && !isDetectorWithinRange(unit->getTilePosition(), 192))
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
