#include "Utilities/Helper.h"
#include "Utilities/Logger.h"
#include "Commander.h"
#include "UnitManager.h"
#include "Squad.h"
#include "SquadManager.h"
#include "UnitManager.h"
#include "UnitCompositionFactory.h"
#include <memory.h>

#include "AttackSquad.h"

using namespace bats;
using namespace std::tr1;

Commander* Commander::mpsInstance = NULL;

Commander::Commander() {
	mpUnitCompositionFactory = NULL;
	mpUnitManager = NULL;
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
	/// @todo implement when we have onMinimapPing event.
	//if (mpSquadWaiting != NULL) {
	//	finishWaitingSquad();
	//}

	if (command == "attack") {
		// Get free units
		std::vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_HasNoSquad);

		// Only add if we have free units
		if (!freeUnits.empty()) {
			// Add the units to the old attack squad if it exists
			shared_ptr<AttackSquad> oldSquad;
			std::map<SquadId, shared_ptr<Squad>>::iterator squadIt = mpSquadManager->begin();
			while (oldSquad == NULL && squadIt != mpSquadManager->end()) {
				oldSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);
			}

			if (oldSquad != NULL) {
				oldSquad->addUnits(freeUnits);
			} else {
				AttackSquad* attackSquad = new AttackSquad(freeUnits);
				mSquadWaiting = attackSquad->getThis();
			}
		}
	} else if (command == "drop") {
		/// @todo drop command
	} else if (command == "harass") {
		/// @todo harass command
	} else if (command == "counter-attack") {
		/// @todo counter-attack command
	} else if (command == "expand") {
		/// @todo expand command
	} else if (command == "move") {
		/// @todo move command
	} else if (command == "scout") {
		/// @todo scout command
	}
	/// @todo abort

	/// @todo remove activating squad directly 
	finishWaitingSquad();

	return true;
}

bool Commander::isCommandAvailable(const std::string& command) const {
	size_t cCommand = mAvailableCommands.count(command);
	return cCommand == 1;
}

void Commander::finishWaitingSquad() {
	if (mSquadWaiting == NULL) {
		return;
	}

	/// @todo finish the waiting squad


	/// @todo implement when we have onMinimapPing event.
	// If we have a path
	
	// No path, do we create a path or do we let the squad create the path?

	mSquadWaiting->activate();
	mSquadWaiting.reset();
}
