#include "SiegeTankAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

SiegeTankAgent::SiegeTankAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "SiegeTankAgent";
}

void SiegeTankAgent::computeActions()
{
	if (Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode))
	{
		// @todo always siege in defensive mode
		int eCnt = enemyGroundUnitsWithinRange(getGroundRange(UnitTypes::Terran_Siege_Tank_Siege_Mode));
		if (eCnt > 0 && !unit->isSieged())
		{
			unit->siege();
			return;
		}
		if (eCnt == 0 && unit->isSieged())
		{
			unit->unsiege();
			return;
		}
	}

	findAndTryAttack();

	//The tank cant move if sieged
	if (!unit->isSieged())
	{
		computeMoveAction();
	}
}
