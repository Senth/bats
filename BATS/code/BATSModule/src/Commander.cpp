#include "Utilities/Helper.h"
#include "Utilities/Logger.h"
#include "Commander.h"
#include "UnitManager.h"
#include "Squad.h"
#include "SquadManager.h"
#include "PlayerArmyManager.h"
#include "AlliedSquad.h"
#include "UnitManager.h"
#include "UnitCompositionFactory.h"
#include "BTHAIModule/Source/CoverMap.h"
#include "BTHAIModule/Source/Profiler.h"
#include <memory.h>

#include "AttackSquad.h"
#include "DropSquad.h"
#include "ScoutSquad.h"
#include "BuildPlanner.h"

using namespace bats;
using namespace std::tr1;
using namespace std;
using namespace BWAPI;

Commander* Commander::msInstance = NULL;

Commander::Commander() {
	mUnitCompositionFactory = NULL;
	mUnitManager = NULL;
	mSquadManager = NULL;
	mAlliedArmyManager = NULL;

	mAlliedArmyManager = PlayerArmyManager::getInstance();
	mSquadManager = SquadManager::getInstance();
	mUnitManager = UnitManager::getInstance();
	mUnitCompositionFactory = UnitCompositionFactory::getInstance();

	initStringToEnums();
}

Commander::~Commander() {
	SAFE_DELETE(mUnitCompositionFactory);

	mSquadManager = NULL;
	mUnitManager = NULL;
	mAlliedArmyManager = NULL;

	msInstance = NULL;
}

Commander* Commander::getInstance() {
	if (msInstance == NULL) {
		msInstance = new Commander();
	}
	return msInstance;
}

void Commander::computeActions() {
	Profiler::getInstance()->start("Commander::update()");

	/// @todo Commander computer actions more complex actions

	computeReactions();
	mSquadManager->update();

	Profiler::getInstance()->end("Commander::update()");
}

void Commander::computeReactions() {
	if (!config::module::PLAYER_REACT) {
		return;
	}

	// Check for active allied squads
	vector<AlliedSquadCstPtr> squads = mAlliedArmyManager->getSquads<AlliedSquad>();
	bool bigActive = false;
	bool smallActive = false;
	for (size_t i = 0; i < squads.size(); ++i) {
		if (squads[i]->isActive()) {
			// Big
			if (squads[i]->isFrontalAttack()) {
				bigActive = true;
			} else {
				smallActive = true;
			}
		}
	}

	
	// Big is active -> we don't have any frontal attack, create frontal attack
	if (bigActive) {
		const AttackSquadPtr& frontalSquad = mSquadManager->getFrontalAttack();

		if (NULL == frontalSquad) {
			issueCommand(Command_Attack, false);
		}
	}

	// Small is active -> Create distraction if we don't have a distraction out
	if (smallActive) {
		const vector<AttackSquadPtr>& distractingAttacks = mSquadManager->getDistractingAttacks();

		if (distractingAttacks.empty()) {
			issueCommand(Command_Drop, false);
		}
	}


	// @todo check for player expanding
}

void Commander::issueCommand(const std::string& command) {
	if (isCommandAvailable(command)) {
		issueCommand(mCommandStringToEnums[command], true);
	}
}

void Commander::issueCommand(Commands command, bool alliedOrdered) {
	if (isAlliedCreatingCommand()) {
		// Allied ordered a new command, finish the old one.
		if (alliedOrdered) {
			finishAlliedCreatingCommand();
		}
		// Don't let command override the allied player already creating an command.
		else {
			return;
		}
	}


	switch(command) {
	case Command_Attack:
		orderAttack(alliedOrdered);
		break;

	case Command_Drop:
		orderDrop(alliedOrdered);
		break;

	case Command_Expand:
		if(BuildPlanner::getInstance()->isExpansionAvailable(BWAPI::Broodwar->self()->getRace().getCenter()))
			BuildPlanner::getInstance()->expand(BWAPI::Broodwar->self()->getRace().getCenter());
		else
			DEBUG_MESSAGE(utilities::LogLevel_Info, "Expansion not available yet");
		break;

	case Command_Scout:
		orderScout(alliedOrdered);
		break;

		/// @todo abort command

	default:
		ERROR_MESSAGE(false, "Commander: Unknown Command type!");
		break;
	}
	
}

bool Commander::isCommandAvailable(const std::string& command) const {
	size_t cCommands = mCommandStringToEnums.count(command);
	return cCommands == 1;
}

void Commander::finishAlliedCreatingCommand() {
	if (mAlliedSquadCommand == NULL) {
		return;
	}

	/// @todo finish the waiting squad


	/// @todo implement when we have onMinimapPing event.
	// If we have a path
	
	// No path, do we create a path or do we let the squad create the path?

	mAlliedSquadCommand->activate();
	mAlliedSquadCommand.reset();
}

#pragma warning(push)
#pragma warning(disable:4100)
void Commander::orderAttack(bool alliedOrdered) {
	/// @todo If we have a frontal attack and allied has a big attack and frontal attack
	/// isn't following any attack. Follow frontal attack now.

	/// @todo Never do a frontal attack if we're under attack (defending)


	// Get free units
	std::vector<UnitAgent*> freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free);

	// Only add if we have free units
	if (!freeUnits.empty()) {
		// Add the units to the old attack squad if it exists
		shared_ptr<AttackSquad> oldSquad;
		map<SquadId, shared_ptr<Squad>>::iterator squadIt = mSquadManager->begin();
		while (oldSquad == NULL && squadIt != mSquadManager->end()) {
			if (squadIt->second->getName() == "AttackSquad") {
				oldSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);
			}
			++squadIt;
		}

		if (oldSquad != NULL) {
			oldSquad->addUnits(freeUnits);
		} else {
			new AttackSquad(freeUnits);
		}
	}
}

void Commander::orderDrop(bool alliedOrdered) {
	/// @todo Check how many attacks we have (frontal, distracting)
	/// Can we create a distracting attack?


	// Get available unit compositions
	vector<UnitAgent*> freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free | UnitFilter_WorkersFree);
	vector<UnitComposition> availableUnitCompositions;
	availableUnitCompositions = mUnitCompositionFactory->getUnitCompositionsByType(freeUnits, UnitComposition_Drop);


	// Create drop
	if (!availableUnitCompositions.empty()) {
		// Choose a random unit composition
		unsigned int randomId = rand() % availableUnitCompositions.size();
		const UnitComposition& chosenComposition = availableUnitCompositions[randomId];

		new DropSquad(freeUnits, chosenComposition);
	}
	// No drops available
	else {
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Commander::createDrop() | No drops available");

		/// @todo print a message to the other player that no drops are available
		/// But only if the player issued the command. Probably if the commander issued
		/// the command a drop shall always be available?
	}
}

void Commander::orderScout(bool alliedOrdered) {
	/// @todo what about existing scout, remove it?
	
	// Get available unit compositions
	//std::vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_WorkersFree);
	//UnitAgent* unit = freeUnits.at(0);
	//freeUnits.clear();
	//freeUnits.push_back(unit);
	//ScoutSquad* pScoutSuad = new ScoutSquad(freeUnits);
	//mSquadWaiting = pScoutSuad->getThis();
	
	// This will return all regular units that is in no squad and all workers that are free (is neither building nor in a squad)
	vector<UnitAgent*> freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free | UnitFilter_WorkersFree);

	// Get all unit compositions that can be created from the specified units.
	// I.e. it will try to fill up all the slots in the unit compositions, those that can be fully
	// filled will be returned in a prioritized order. E.g. if we have 10 free workers, 8 marines,
	// and 2 medics. It can fill the scout composition with an SCV it will only return it. If we
	// instead have 10 free workers, 8 marines, 2 medics, 6 wraiths, and 2 Vultures it can fill
	// all three compositions; these will then be returned sorted by the highest priority.
	// Meaning if Wraith has priority 3, Vulture priority 2, and SCV priority 1,
	// element [0] -> Wrait Unit composition, [1] -> Vulture, [2] -> SCV.
	vector<UnitComposition> availableUnitCompositions;
	availableUnitCompositions = mUnitCompositionFactory->getUnitCompositionsByType(freeUnits, UnitComposition_Scout);

	// Create a squad with the highest composition.
	if (!availableUnitCompositions.empty()) {
		new ScoutSquad(freeUnits, true, availableUnitCompositions[0]);
	}
}
#pragma warning(pop)

void Commander::initStringToEnums() {
	mCommandStringToEnums["attack"] = Command_Attack;
	mCommandStringToEnums["drop"] = Command_Drop;
	mCommandStringToEnums["expand"] = Command_Expand;
	mCommandStringToEnums["scout"] = Command_Scout;

	if (mCommandStringToEnums.size() != Command_Lim) {
		ERROR_MESSAGE(false, "Commander: Command String to enum does not have same size as enumerations!");
	}
}

bool Commander::isAlliedCreatingCommand() const {
	return NULL != mAlliedSquadCommand;
}