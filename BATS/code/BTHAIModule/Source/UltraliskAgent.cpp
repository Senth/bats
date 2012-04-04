#include "UltraliskAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

UltraliskAgent::UltraliskAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "UltraliskAgent";
	//Broodwar->printf("UltraliskAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void UltraliskAgent::computeActions()
{
	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}
