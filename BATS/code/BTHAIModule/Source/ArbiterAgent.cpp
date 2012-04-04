#include "ArbiterAgent.h"
#include "PFManager.h"
#include "AgentManager.h"

using namespace BWAPI;
using namespace std;

ArbiterAgent::ArbiterAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "ArbiterAgent";
	//Broodwar->printf("ArbiterAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void ArbiterAgent::computeActions()
{
	if (chargeShields())
	{
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, true);
}
