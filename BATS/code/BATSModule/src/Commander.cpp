#include "Utilities/Helper.h"
#include "Utilities/Logger.h"
#include "Commander.h"
#include "Squad.h"
#include "SquadManager.h"
#include "UnitCompositionFactory.h"

using namespace bats;

Commander* Commander::mpsInstance = NULL;

Commander::Commander() {
	mpUnitCompositionFactory = NULL;
	mpSquadWaiting = NULL;
	mpSquadManager = NULL;

	mpSquadManager = SquadManager::getInstance();
	mpUnitCompositionFactory = UnitCompositionFactory::getInstance();
}

Commander::~Commander() {
	SAFE_DELETE(mpUnitCompositionFactory);

	// Delete squads
	SAFE_DELETE(mpSquadManager);
	SAFE_DELETE(mpSquadWaiting);

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
		/// @todo attack
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

	return true;
}

void Commander::finishWaitingSquad() {
	/// @todo finish the waiting squad


	/// @todo implement when we have onMinimapPing event.
	// If we have a path
	
	// No path, do we create a path or do we let the squad create the path?

	mpSquadManager->addSquad(mpSquadWaiting);
	mpSquadWaiting = NULL;
}