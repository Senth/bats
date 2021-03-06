#include "GhostAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

GhostAgent::GhostAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "GhostAgent";
}

void GhostAgent::computeActions()
{
	/// @todo use const variables instead of 384 and 192 (what does 192 mean?)
	int eCnt = enemyGroundAttackingUnitsWithinRange(384) + enemyAirAttackingUnitsWithinRange(384); //384 = range of tank in siege mode
	
	TechType cloak = TechTypes::Personnel_Cloaking;
	if (Broodwar->self()->hasResearched(cloak))
	{
		if (!unit->isCloaked() && eCnt > 0 && !isDetectorWithinRange(192))
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

	findAndTryAttack();
	computeMoveAction();
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
