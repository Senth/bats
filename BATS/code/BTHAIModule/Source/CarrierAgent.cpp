#include "CarrierAgent.h"
#include "PFManager.h"
#include "AgentManager.h"

using namespace BWAPI;
using namespace std;

CarrierAgent::CarrierAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "CarrierAgent";
	//Broodwar->printf("CarrierAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void CarrierAgent::computeActions()
{
	if (chargeShields())
	{
		return;
	}

	if (Broodwar->canMake(unit, UnitTypes::Protoss_Interceptor))
	{
		unit->train(UnitTypes::Protoss_Interceptor);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, true);
}
