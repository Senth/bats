#include "ObserverAgent.h"
#include "PFManager.h"
#include "AgentManager.h"

using namespace BWAPI;
using namespace std;

ObserverAgent::ObserverAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "ObserverAgent";
	//Broodwar->printf("ObserverAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void ObserverAgent::computeActions()
{
	bool defensive = true;
	PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
}
