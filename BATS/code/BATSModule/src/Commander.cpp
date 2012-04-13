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
#include "DropSquad.h"

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
		createAttack();
	} else if (command == "drop") {
		createDrop();
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

void Commander::createAttack() {
	///@ todo handle already existing waiting squad.
	if (NULL != mSquadWaiting) {
		finishWaitingSquad();
	}

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
}

void Commander::createDrop() {
	/// @todo handle already existing waiting squad
	if (NULL != mSquadWaiting) {
		finishWaitingSquad();
	}

	// Get available unit compositions
	std::vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_HasNoSquad | UnitFilter_WorkersNoSquad);
	std::vector<UnitComposition> availableUnitCompositions;
	availableUnitCompositions = mpUnitCompositionFactory->getUnitCompositionsByType(freeUnits, UnitComposition_Drop);

	// Create drop
	if (!availableUnitCompositions.empty()) {
		// Choose a random unit composition
		unsigned int randomId = rand() % availableUnitCompositions.size();
		const UnitComposition& chosenComposition = availableUnitCompositions[randomId];

		DropSquad* pDropSquad = new DropSquad(freeUnits, chosenComposition);
		mSquadWaiting = pDropSquad->getThis();
	}
	// No drops available
	else {
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Commander::createDrop() | No drops available");

		/// @todo print a message to the other player that no drops are available
		/// But only if the player issued the command. Probably if the commander issued
		/// the command a drop shall always be available?
	}
}