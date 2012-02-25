#include "GhostAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

GhostAgent::GhostAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "GhostAgent";
	//Broodwar->printf("GhostAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void GhostAgent::computeActions()
{
	int eCnt = enemyGroundAttackingUnitsWithinRange(unit->getTilePosition(), 384) + enemyAirAttackingUnitsWithinRange(unit->getTilePosition(), 384); //384 = range of tank in siege mode
	
	TechType cloak = TechTypes::Personnel_Cloaking;
	if (Broodwar->self()->hasResearched(cloak))
	{
		if (!unit->isCloaked() && eCnt > 0 && !isDetectorWithinRange(unit->getTilePosition(), 192))
		{
			if (unit->getEnergy() > 25)
			{
				unit->useTech(cloak);
				//Broodwar->printf("[%d] Ghost used cloaking", unitID);
				return;
			}
		}
	}

	TechType lockdown = TechTypes::Lockdown;
	if (Broodwar->self()->hasResearched(lockdown))
	{
		if (unit->getEnergy() >= 100)
		{
			Unit* target = findLockdownTarget();
			if (target != NULL)
			{
				Broodwar->printf("[%d] Used Lockdown on [%d] %s", unitID, target->getID(), target->getType().getName().c_str());
				unit->useTech(lockdown, target);
				return;
			}
		}
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}

Unit* GhostAgent::findLockdownTarget()
{
	int fCnt = friendlyUnitsWithinRange(224);
	if (fCnt < 2)
	{
		//If we dont have any attacking units nearby,
		//dont bother with lockdown.
		return NULL;
	}

	int maxRange = getGroundRange();

	Unit* target = NULL;
	int cTargetVal = 0;

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->getType().isMechanical() && !(*i)->getLockdownTimer() == 0 && !(*i)->getType().isBuilding())
		{
			int targetVal = (*i)->getType().destroyScore();
			if (targetVal >= 200 && targetVal > cTargetVal)
			{
				target = (*i);
				cTargetVal = targetVal;
			}
		}
	}

	return target;
}
