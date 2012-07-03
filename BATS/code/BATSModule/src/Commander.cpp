#include "Utilities/Helper.h"
#include "Utilities/Logger.h"
#include "Commander.h"
#include "UnitManager.h"
#include "Squad.h"
#include "SquadManager.h"
#include "AlliedArmyManager.h"
#include "AlliedSquad.h"
#include "UnitManager.h"
#include "UnitCompositionFactory.h"
#include "BTHAIModule/Source/CoverMap.h"
#include <memory.h>

#include "AttackSquad.h"
#include "DropSquad.h"
#include "ScoutSquad.h"

using namespace bats;
using namespace std::tr1;
using namespace std;
using namespace BWAPI;

Commander* Commander::mpsInstance = NULL;

Commander::Commander() {
	mpUnitCompositionFactory = NULL;
	mpUnitManager = NULL;
	mpSquadManager = NULL;
	mpAlliedArmyManager = NULL;

	mpAlliedArmyManager = AlliedArmyManager::getInstance();
	mpSquadManager = SquadManager::getInstance();
	mpUnitManager = UnitManager::getInstance();
	mpUnitCompositionFactory = UnitCompositionFactory::getInstance();

	mAvailableCommands.insert("abort");
	mAvailableCommands.insert("attack");
	//mAvailableCommands.insert("counter-attack");
	mAvailableCommands.insert("drop");
	mAvailableCommands.insert("expand");
	//mAvailableCommands.insert("move");
	mAvailableCommands.insert("scout");
}

Commander::~Commander() {
	SAFE_DELETE(mpUnitCompositionFactory);

	mpSquadManager = NULL;
	mpUnitManager = NULL;
	mpAlliedArmyManager = NULL;
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

	computeReactions();
	mpSquadManager->computeActions();
}

void Commander::computeReactions() {
	// Allied is outside home with big attack
	AlliedSquadCstPtr bigSquad = mpAlliedArmyManager->getBigSquad();
	if (NULL != bigSquad && bigSquad->getState() != AlliedSquad::State_Idle) {
		// Check for our frontal attack and if it's following an allied squad or not
		bool haveFrontalAttack = false;
		bool followingAlliedSquad = false;
		SquadIt squadIt = mpSquadManager->begin();
		while (!haveFrontalAttack && squadIt != mpSquadManager->end()) {
			shared_ptr<AttackSquad> attackSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);
			if (NULL != attackSquad) {
				if (!attackSquad->isDistracting()) {
					haveFrontalAttack = true;

					if (attackSquad->isFollowingAlliedSquad()) {
						followingAlliedSquad = true;
					}
				}
			}

			++squadIt;
		}

		// Create frontal attack
		if (!haveFrontalAttack) {
			issueCommand("attack");
		}
		// Create distraction
		else if (!followingAlliedSquad) {
			issueCommand("drop");
		}
	}
	/// @todo Allied is outside home with any attack
		// We don't have any attack -> Create frontal or distracting attack
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
		createScout();
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
	// Handle already existing waiting squad.
	if (NULL != mSquadWaiting) {
		finishWaitingSquad();
	}


	/// @todo If we have a frontal attack and allied has a big attack and frontal attack
	/// isn't following any attack. Follow frontal attack now.


	/// @todo Never do a frontal attack if we're under attack (defending)


	// Get free units
	std::vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_HasNoSquad);

	// Only add if we have free units
	if (!freeUnits.empty()) {
		// Add the units to the old attack squad if it exists
		shared_ptr<AttackSquad> oldSquad;
		std::map<SquadId, shared_ptr<Squad>>::iterator squadIt = mpSquadManager->begin();
		while (oldSquad == NULL && squadIt != mpSquadManager->end()) {
			if (squadIt->second->getName() == "AttackSquad") {
				oldSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);
			}
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


	/// @todo Check how many attacks we have (frontal, distracting)
	/// Can we create a distracting attack?


	// Get available unit compositions
	std::vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_HasNoSquad | UnitFilter_WorkersFree);
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

void Commander::createScout() {
	/// @todo handle already existing waiting squad
	if (NULL != mSquadWaiting) {
		finishWaitingSquad();
	}
	/*
	// Get available unit compositions
	std::vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_HasNoSquad);
	if(freeUnits.size() <= 0)
		freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_WorkersAll);
	UnitAgent* unit = freeUnits.at(0);
	freeUnits.clear();
	freeUnits.push_back(unit);
	ScoutSquad* pScoutSuad = new ScoutSquad(freeUnits);
	mSquadWaiting = pScoutSuad->getThis();
	*/
		// This will return all regular units that is in no squad and all workers that are free (is neither building nor in a squad)
	std::vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_HasNoSquad | UnitFilter_WorkersFree);

	// Get all unit compositions that can be created from the specified units.
	// I.e. it will try to fill up all the slots in the unit compositions, those that can be fully
	// filled will be returned in a prioritized order. E.g. if we have 10 free workers, 8 marines,
	// and 2 medics. It can fill the scout composition with an SCV it will only return it. If we
	// instead have 10 free workers, 8 marines, 2 medics, 6 wraiths, and 2 Vultures it can fill
	// all three compositions; these will then be returned sorted by the highest priority.
	// Meaning if Wraith has priority 3, Vulture priority 2, and SCV priority 1,
	// element [0] -> Wrait Unit composition, [1] -> Vulture, [2] -> SCV.
	std::vector<UnitComposition> availableUnitCompositions;
	availableUnitCompositions = mpUnitCompositionFactory->getUnitCompositionsByType(freeUnits, UnitComposition_Scout);

	// Create a squad with the highest composition.
	if (!availableUnitCompositions.empty()) {
		ScoutSquad* pScoutSquad = new ScoutSquad(freeUnits, true, availableUnitCompositions[0]);
		mSquadWaiting = pScoutSquad->getThis();
	}
}

#pragma warning(push)	// Disabled until the squad is actually used.
#pragma warning(disable:4100)
TilePosition Commander::getRetreatPosition(const shared_ptr<Squad>& squad) const {
	/// @todo check if the squad shall stay and fight instead of retreating

	/// @todo get a better retreat position than a good choke point

	return CoverMap::getInstance()->findChokepoint();
}
#pragma warning(pop)