#include "UnitManager.h"
#include "SquadManager.h"
#include "Squad.h"
#include "BTHAIModule/Source/WorkerAgent.h"
#include <cassert>
#include <memory.h>

using namespace bats;
using std::tr1::shared_ptr;

UnitManager::UnitManager() {
	mpSquadManager = NULL;

	mpSquadManager = SquadManager::getInstance();
}

UnitManager::~UnitManager() {
	// Does nothing, AgentManager's destructor already takes
	// care of setting instance = NULL;
}

UnitManager* UnitManager::getInstance() {
	if (mpsInstance == NULL) {
		mpsInstance = new UnitManager();
	}
	UnitManager* pUnitManager = dynamic_cast<UnitManager*>(mpsInstance);
	
	// assert: Forgot to call UnitManager::getInstance() before AgentManager::getInstance().
	assert(pUnitManager != NULL);
	
	return pUnitManager;
}

std::vector<const UnitAgent*> UnitManager::getUnitsByFilter(int filter) const {
	std::vector<UnitAgent*> units = const_cast<UnitManager*>(this)->getUnitsByFilter(filter);
	
	std::vector<const UnitAgent*>& foundUnits = *reinterpret_cast<std::vector<const UnitAgent*>* >(&units);
	return foundUnits;
}

std::vector<UnitAgent*> UnitManager::getUnitsByFilter(int filter) {
	std::vector<UnitAgent*> foundUnits;
	for (size_t i = 0; i < mAgents.size(); ++i) {
		UnitAgent* pUnitAgent = dynamic_cast<UnitAgent*>(mAgents[i]);
		if (NULL != pUnitAgent) {
			// Automatically skip constructing and dead units
			if (pUnitAgent->isBeingBuilt() || !pUnitAgent->isAlive()) {
				continue;
			}

			bool addUnit = false;

			// Worker units
			if (pUnitAgent->getUnitType().isWorker()) {
				// All workers including those that are building
				if (filter & UnitFilter_WorkersAll) {
					addUnit = true;
				}

				// Only add free workers (in no squad and not building)
				if (filter & UnitFilter_WorkersFree) {
					WorkerAgent* pWorkerAgent = dynamic_cast<WorkerAgent*>(pUnitAgent);
					if (NULL != pWorkerAgent &&
						!pWorkerAgent->isConstructing() &&
						pWorkerAgent->getSquadId().isInvalid())
					{
						addUnit = true;
					}
				}
			}
			// Non-worker units
			else {
				if (filter & UnitFilter_HasNoSquad) {
					if (pUnitAgent->getSquadId().isInvalid()) {
						addUnit = true;
					}
				}

				if (filter & UnitFilter_InDisbandableSquad) {
					SquadId squadId = pUnitAgent->getSquadId();

					if (mpSquadManager->getSquad(squadId)->isDisbandable()) {
						addUnit = true;
					}
				}

				if (filter & UnitFilter_UnitsAll) {
					addUnit = true;
				}
			}

			if (filter == UnitFilter_NoFilter) {
				addUnit = true;
			}

			if (addUnit) {
				foundUnits.push_back(pUnitAgent);
			}
		}
	}

	return foundUnits;
}

void UnitManager::onAgentDestroyed(BaseAgent* destroyedAgent) {
	AgentManager::onAgentDestroyed(destroyedAgent);

	// Is it a UnitAgent?
	UnitAgent* pUnitAgent = dynamic_cast<UnitAgent*>(destroyedAgent);
	
	if (pUnitAgent != NULL) {
		// Delete from squad if the unit had any
		SquadId unitSquad = pUnitAgent->getSquadId();
		if (unitSquad.isValid()) {
			SquadRef pSquad = mpSquadManager->getSquad(unitSquad);
			pSquad->removeUnit(pUnitAgent);
		}
	}
}