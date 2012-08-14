#include "UnitManager.h"
#include "SquadManager.h"
#include "Squad.h"
#include "DefenseManager.h"
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
	UnitManager* unitManager = dynamic_cast<UnitManager*>(mpsInstance);
	
	// assert: Forgot to call UnitManager::getInstance() before AgentManager::getInstance().
	assert(unitManager != NULL);
	
	return unitManager;
}

std::vector<const UnitAgent*> UnitManager::getUnitsByFilter(int filter) const {
	const std::vector<UnitAgent*>& units = const_cast<UnitManager*>(this)->getUnitsByFilter(filter);
	
	const std::vector<const UnitAgent*>& foundUnits = *reinterpret_cast<const std::vector<const UnitAgent*>* >(&units);
	return foundUnits;
}

std::vector<UnitAgent*> UnitManager::getUnitsByFilter(int filter) {
	std::vector<UnitAgent*> foundUnits;
	for (size_t i = 0; i < mAgents.size(); ++i) {
		UnitAgent* unitAgent = dynamic_cast<UnitAgent*>(mAgents[i]);
		if (NULL != unitAgent) {
			// Automatically skip constructing and dead units
			if (unitAgent->isBeingBuilt() || !unitAgent->isAlive()) {
				continue;
			}

			bool addUnit = false;

			// Worker units
			if (unitAgent->getUnitType().isWorker()) {
				// All workers including those that are building
				if (filter & UnitFilter_WorkersAll) {
					addUnit = true;
				}

				// Only add free workers (in no squad and not building)
				if (filter & UnitFilter_WorkersFree) {
					WorkerAgent* pWorkerAgent = dynamic_cast<WorkerAgent*>(unitAgent);
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
				if (filter & UnitFilter_HasNoSquad | UnitFilter_Free) {
					if (unitAgent->getSquadId().isInvalid()) {
						addUnit = true;
					}
				}

				if (filter & UnitFilter_InDisbandableSquad) {
					SquadId squadId = unitAgent->getSquadId();
					if (squadId.isValid()) {
						SquadPtr pSquad = mpSquadManager->getSquad(squadId);

						if (NULL != pSquad && pSquad->isDisbandable()) {
							addUnit = true;
						}
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
				foundUnits.push_back(unitAgent);
			}
		}
	}

	// Add free units from defense manager
	if (filter & UnitFilter_Free) {
		const std::vector<UnitAgent*>& defenseUnits = DefenseManager::getInstance()->getFreeUnits();
		foundUnits.insert(foundUnits.end(), defenseUnits.begin(), defenseUnits.end());
	}

	return foundUnits;
}

void UnitManager::onAgentDestroyed(BaseAgent* destroyedAgent) {
	AgentManager::onAgentDestroyed(destroyedAgent);

	// Is it a UnitAgent?
	UnitAgent* unitAgent = dynamic_cast<UnitAgent*>(destroyedAgent);
	
	if (unitAgent != NULL) {
		// Delete from squad if the unit had any
		SquadId unitSquad = unitAgent->getSquadId();
		if (unitSquad.isValid()) {
			SquadRef pSquad = mpSquadManager->getSquad(unitSquad);
			pSquad->removeUnit(unitAgent);
		}
	}
}