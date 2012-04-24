#include "RefineryAgent.h"
#include "WorkerAgent.h"
#include "AgentManager.h"

using namespace BWAPI;
using namespace std;

RefineryAgent::RefineryAgent(Unit* mUnit) : StructureAgent(mUnit)
{
	agentType = "RefineryAgent";
}

void RefineryAgent::computeActions()
{
	for (int i = 0; i < (int)assignedWorkers.size(); i++)
	{
		if (!assignedWorkers.at(i)->isAlive())
		{
			assignedWorkers.erase(assignedWorkers.begin() + i);
			return;
		}
		if (!assignedWorkers.at(i)->getUnit()->isGatheringGas())
		{
			assignedWorkers.erase(assignedWorkers.begin() + i);
			return;
		}
	}

	if (assignedWorkers.size() <= 3)
	{
		if (!unit->isBeingConstructed() && unit->getPlayer()->getID() == Broodwar->self()->getID())
		{
			WorkerAgent* worker = (WorkerAgent*)AgentManager::getInstance()->findClosestFreeWorker(unit->getTilePosition());
			if (worker != NULL)
			{
				worker->getUnit()->rightClick(unit);
				worker->setState(WorkerAgent::GATHER_GAS);
				assignedWorkers.push_back(worker);
			}
		}
	}
}
