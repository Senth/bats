#include "NexusAgent.h"
#include "AgentManager.h"
#include "WorkerAgent.h"
#include "PFManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "ResourceManager.h"

using namespace BWAPI;
using namespace std;

NexusAgent::NexusAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "NexusAgent";
	//Broodwar->printf("New base created at (%d,%d)", unit->getTilePosition().x(), unit->getTilePosition().y());

	hasSentWorkers = false;
	if (AgentManager::getInstance()->countNoUnits(UnitTypes::Protoss_Nexus) == 0)
	{
		//We dont do this for the first Nexus.
		hasSentWorkers = true;
	}
	
	bats::BuildPlanner::getInstance()->commandCenterBuilt();
}

void NexusAgent::computeActions()
{
	if (!hasSentWorkers)
	{
		if (!unit->isBeingConstructed())
		{
			sendWorkers();
			hasSentWorkers = true;

			bats::BuildPlanner::getInstance()->addRefinery();

			if (AgentManager::getInstance()->countNoUnits(UnitTypes::Protoss_Forge) > 0)
			{
				bats::BuildPlanner::getInstance()->addBuildingFirst(UnitTypes::Protoss_Pylon);
				bats::BuildPlanner::getInstance()->addBuildingFirst(UnitTypes::Protoss_Photon_Cannon);
			}
		}
	}

	if (!unit->isIdle())
	{
		//Already doing something
		return;
	}

	if (ResourceManager::getInstance()->needWorker())
	{
		UnitType worker = Broodwar->self()->getRace().getWorker();
		if (canBuild(worker))
		{
			unit->train(worker);
		}
	}
}
