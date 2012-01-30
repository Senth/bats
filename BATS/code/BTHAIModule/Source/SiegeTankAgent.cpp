#include "SiegeTankAgent.h"
#include "AgentManager.h"
#include "PFManager.h"
#include "TargetingAgent.h"

SiegeTankAgent::SiegeTankAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "SiegeTankAgent";
	//Broodwar->printf("SiegeTankAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
}

void SiegeTankAgent::computeActions()
{
	if (Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode))
	{
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

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	//The tank cant move if sieged
	if (!unit->isSieged())
	{
		PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
	}
}
