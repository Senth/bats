#include "Utilities/Helper.h"
#include "Utilities/Logger.h"
#include "Commander.h"
#include "UnitManager.h"
#include "Squad.h"
#include "SquadManager.h"
#include "UnitManager.h"
#include "UnitCompositionFactory.h"

#include "AttackSquad.h"

using namespace bats;

Commander* Commander::mpsInstance = NULL;

Commander::Commander() {
	mpUnitCompositionFactory = NULL;
	mpUnitManager = NULL;
	mpSquadWaiting = NULL;
	mpSquadManager = NULL;

	mpSquadManager = SquadManager::getInstance();
	mpUnitManager = UnitManager::getInstance();
	mpUnitCompositionFactory = UnitCompositionFactory::getInstance();

	mAvailableCommands.insert("abort");
	mAvailableCommands.insert("attack");
	mAvailableCommands.insert("counter-attack");
	mAvailableCommands.insert("drop");
	mAvailableCommands.insert("expand");
	mAvailableCommands.insert("move");
	mAvailableCommands.insert("scout");
}

Commander::~Commander() {
	SAFE_DELETE(mpUnitCompositionFactory);

	// Delete squads
	SAFE_DELETE(mpSquadWaiting);

	mpSquadManager = NULL;
	mpUnitManager = NULL;
	mpsInstance = NULL;
}

Commander* Commander::getInstance() {
	if (mpsInstance == NULL) {
		mpsInstance = new Commander();
	}
	return mpsInstance;
}

void Commander::computeActions() {
	/// @todo Commander computer actions more complex actions

	mpSquadManager->computeActions();
}

bool Commander::issueCommand(const std::string& command) {
	/// @todo Commander try to issue command.

	/// @todo implement when we have onMinimapPing event.
	//if (mpSquadWaiting != NULL) {
	//	finishWaitingSquad();
	//}

	if (command == "attack") {
		// Get free units
		std::vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_NoSquad);

		// Only add if we have free units
		if (!freeUnits.empty()) {
			// Add the units to the old attack squad if it exists
			AttackSquad* pOldSquad = NULL;
			std::map<SquadId, Squad*>::iterator squadIt = mpSquadManager->begin();
			while (pOldSquad == NULL && squadIt != mpSquadManager->end()) {
				pOldSquad = dynamic_cast<AttackSquad*>(squadIt->second);
			}

			if (pOldSquad != NULL) {
				pOldSquad->addUnits(freeUnits);
			} else {
				mpSquadWaiting = new AttackSquad(freeUnits);
			}
		}
	} else if (command == "drop") {
		/// @todo drop
	} else if (command == "harass") {
		/// @todo harass
	} else if (command == "counter-attack") {
		/// @todo counter-attack
	} else if (command == "expand") {
		/// @todo expand
	} else if (command == "move") {
		/// @todo move
	} else if (command == "scout") {
		/// @todo scout
	}
	/// @todo abort

	/// @todo remove adding squad directly to squad manager
	finishWaitingSquad();

	return true;
}

bool Commander::isCommandAvailable(const std::string& command) const {
	size_t cCommand = mAvailableCommands.count(command);
	return cCommand == 1;
}

void Commander::finishWaitingSquad() {
	if (mpSquadWaiting == NULL) {
		return;
	}

	/// @todo finish the waiting squad


	/// @todo implement when we have onMinimapPing event.
	// If we have a path
	
	// No path, do we create a path or do we let the squad create the path?

	mpSquadManager->addSquad(mpSquadWaiting);
	mpSquadWaiting = NULL;
}
