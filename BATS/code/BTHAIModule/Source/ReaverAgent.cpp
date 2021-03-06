#include "ReaverAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

ReaverAgent::ReaverAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "ReaverAgent";
	//Broodwar->printf("ReaverAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void ReaverAgent::computeActions()
{
	int maxLoad = 5;
	if (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Reaver_Capacity) > 0)
	{
		maxLoad = 10;
	}

	if(unit->getScarabCount() < maxLoad)
	{
		unit->train(UnitTypes::Protoss_Scarab);
		return;
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}
