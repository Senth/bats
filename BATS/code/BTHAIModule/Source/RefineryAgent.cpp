#include "RefineryAgent.h"
#include "WorkerAgent.h"
#include "AgentManager.h"
#include "BATSModule/include/SelfClassifier.h"
#include <BWAPI/Unit.h>

using namespace BWAPI;
using namespace std;

RefineryAgent::RefineryAgent(Unit* mUnit) : StructureAgent(mUnit)
{
	agentType = "RefineryAgent";
}

void RefineryAgent::computeActions()
{
	handleUnderAttack();

	for (size_t i = 0; i < assignedWorkers.size(); i++)
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

	size_t maxSize;
	if (AgentManager::getInstance()->getWorkerCount() < 8) {
		maxSize = 0;
	}
	else if (bats::SelfClassifier::getInstance()->isHighOnGas()) {
		maxSize = 3;
	}
	else {
		maxSize = 4;
	}

	// Remove assigned worker if too big
	if (assignedWorkers.size() > maxSize) {
		assignedWorkers.back()->setState(WorkerAgent::GATHER_MINERALS);
		assignedWorkers.erase(assignedWorkers.begin() + assignedWorkers.size() - 1);
	}

	if (assignedWorkers.size() < maxSize)
	{
		if (!unit->isBeingConstructed() && unit->getPlayer() == Broodwar->self())
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
