#include "GuardianAgent.h"
#include "PFManager.h"
#include "Commander.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

GuardianAgent::GuardianAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "GuardianAgent";
	//Broodwar->printf("GuardianAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void GuardianAgent::computeActions()
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

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	bool defensive = useDefensiveMode();
	PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
}
