#include "UnitManager.h"
#include "SquadManager.h"
#include "Squad.h"
#include <cassert>

using namespace bats;

UnitManager::UnitManager() {
	mpSquadManager = NULL;

	mpSquadManager = SquadManager::getInstance();
}

UnitManager::~UnitManager() {
	// Does nothing, AgentManager's destructor already takes
	// care of setting instance = NULL;
}

UnitManager* UnitManager::getInstance() {
	if (instance == NULL) {
		instance = new UnitManager();
	}
	UnitManager* pUnitManager = dynamic_cast<UnitManager*>(instance);
	
	// assert: Forgot to call UnitManager::getInstance() before AgentManager::getInstance().
	assert(pUnitManager != NULL);
	
	return pUnitManager;
}

std::vector<UnitAgent*> UnitManager::getUnitsByFilter(int filter) {
	std::vector<const UnitAgent*> constUnits;
	constUnits = const_cast<const UnitManager*>(this)->getUnitsByFilter(filter);
	
	std::vector<UnitAgent*>& foundUnits = *reinterpret_cast<std::vector<UnitAgent*>* >(&constUnits);
	return foundUnits;
}

std::vector<const UnitAgent*> UnitManager::getUnitsByFilter(int filter) const {
	std::vector<const UnitAgent*> foundUnits;
	for (size_t i = 0; i < agents.size(); ++i) {
		UnitAgent* pUnitAgent = dynamic_cast<UnitAgent*>(agents[i]);
		if (NULL != pUnitAgent) {
			bool addUnit = false;

			if (filter & UnitFilter_NoSquad) {
				if (pUnitAgent->getSquadId().isInvalid()) {
					addUnit = true;
				}
			}

			if (filter & UnitFilter_DisbandableSquad) {
				SquadId squadId = pUnitAgent->getSquadId();

				if (mpSquadManager->getSquad(squadId)->isDisbandable()) {
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