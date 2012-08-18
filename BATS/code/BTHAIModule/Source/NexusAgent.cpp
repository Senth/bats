#include "NexusAgent.h"
#include "AgentManager.h"
#include "WorkerAgent.h"
#include "PFManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "ResourceManager.h"

using namespace BWAPI;
using namespace std;

NexusAgent::NexusAgent(Unit* mUnit) : StructureAgent(mUnit)
{
	agentType = "NexusAgent";
	hasSentWorkers = false;
	if (AgentManager::getInstance()->countNoUnits(UnitTypes::Protoss_Nexus) == 0)
	{
		//We dont do this for the first Nexus.
		hasSentWorkers = true;
	}
	
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
				bats::BuildPlanner::getInstance()->addItemFirst(UnitTypes::Protoss_Pylon);
				bats::BuildPlanner::getInstance()->addItemFirst(UnitTypes::Protoss_Photon_Cannon);
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
