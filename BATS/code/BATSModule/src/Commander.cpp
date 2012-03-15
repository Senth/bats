#include "Utilities/Helper.h"
#include "Utilities/Logger.h"
#include "Commander.h"
#include "Squad.h"
#include "UnitCompositionFactory.h"

using namespace bats;

Commander* Commander::mpsInstance = NULL;

Commander::Commander() {
	mpUnitCompositionFactory = NULL;
	mpSquadWaiting = NULL;

	mpUnitCompositionFactory = UnitCompositionFactory::getInstance();
}

Commander::~Commander() {
	SAFE_DELETE(mpUnitCompositionFactory);

	// Delete squads
	for (size_t i = 0; i < mSquads.size(); ++i) {
		SAFE_DELETE(mSquads[i]);
	}
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

	for (size_t i = 0; i < mSquads.size(); ++i) {
		mSquads[i]->computeActions();
	}
}

bool Commander::issueCommand(const std::string& command) {
	/// @todo Commander try to issue command.

	if (mpSquadWaiting != NULL) {
		finishWaitingSquad();
	}

	if (command == "attack") {
		// TODO
	} else if (command == "drop") {
		// TODO
	} else if (command == "harass") {
		// TODO
	}

	return true;
}

void Commander::finishWaitingSquad() {
	/// @todo finish the waiting squad

	// If we have a path
	
	// No path, do we create a path or do we let the squad create the path?

	mSquads.push_back(mpSquadWaiting);
	mpSquadWaiting = NULL;
}