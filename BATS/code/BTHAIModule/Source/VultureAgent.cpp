#include "VultureAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "Commander.h"
#include "TargetingAgent.h"

VultureAgent::VultureAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "VultureAgent";
	//Broodwar->printf("VultureAgent created (%s)", unit->getType().getName().c_str());
	mineDropFrame = 0;

	goal = TilePosition(-1, -1);
}

void VultureAgent::computeActions()
{
	Squad* sq = Commander::getInstance()->getSquad(squadID);
	if (sq != NULL)
	{
		if (sq->isKite())
		{
			computeKitingActions();
			return;
		}
	}

	int eCnt = enemyGroundAttackingUnitsWithinRange(unit->getTilePosition(), 320);
	if (eCnt > 0)
	{
		int framesSinceDrop = Broodwar->getFrameCount() - mineDropFrame;
		if (unit->getSpiderMineCount() > 0 && framesSinceDrop >= 100)
		{
			//Broodwar->printf("[%d] dropped spider mine", unitID);
			unit->useTech(TechTypes::Spider_Mines, unit->getPosition());
			mineDropFrame = Broodwar->getFrameCount();
			return;
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
