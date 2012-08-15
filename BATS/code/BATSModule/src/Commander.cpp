#include "Commander.h"
#include "UnitManager.h"
#include "Squad.h"
#include "SquadManager.h"
#include "PlayerArmyManager.h"
#include "AlliedSquad.h"
#include "UnitManager.h"
#include "IntentionWriter.h"
#include "UnitCompositionFactory.h"
#include "DefenseManager.h"
#include "SelfClassifier.h"
#include "AttackSquad.h"
#include "DropSquad.h"
#include "ScoutSquad.h"
#include "BuildPlanner.h"
#include "GameTime.h"

#include "BTHAIModule/Source/CoverMap.h"
#include "BTHAIModule/Source/Profiler.h"

#include "Utilities/Helper.h"
#include "Utilities/Logger.h"


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
	mDefenseManager = NULL;
	mIntentionWriter = NULL;
	mSelfClassifier = NULL;
	mGameTime = NULL;

	mAlliedArmyManager = PlayerArmyManager::getInstance();
	mSquadManager = SquadManager::getInstance();
	mUnitManager = UnitManager::getInstance();
	mUnitCompositionFactory = UnitCompositionFactory::getInstance();
	mDefenseManager = DefenseManager::getInstance();
	mIntentionWriter = IntentionWriter::getInstance();
	mSelfClassifier = SelfClassifier::getInstance();
	mGameTime = GameTime::getInstance();

	mFrameCallLast = -config::frame_distribution::COMMANDER;
	mExpansionTimeLast = 0.0;

	initStringToEnums();
}

Commander::~Commander() {
	SAFE_DELETE(mUnitCompositionFactory);

	msInstance = NULL;
}

Commander* Commander::getInstance() {
	if (msInstance == NULL) {
		msInstance = new Commander();
	}
	return msInstance;
}

void Commander::computeActions() {
	// Don't call too often
	if (mGameTime->getFrameCount(mFrameCallLast) < config::frame_distribution::COMMANDER) {
		return;
	}
	mFrameCallLast = mGameTime->getFrameCount();

	Profiler::getInstance()->start("Commander::update()");

	computeOwnReactions();
	computeAlliedReactions();

	Profiler::getInstance()->end("Commander::update()");
}

void Commander::computeOwnReactions() {
	if (!config::module::OWN_REACT) {
		return;
	}

	// EXPAND
	// -> When attacking, workers saturated, or minerals running low in an expansion
	// Only expand if we haven't expanded for a while, and set how many maximum numbers of expansions
	// we are allowed to have.
	if (!mSelfClassifier->isExpanding() &&
		mSelfClassifier->getActiveExpansionCount() < config::commander::EXPANSION_ACTIVE_MAX &&
		mSelfClassifier->getLastExpansionStartTime() > config::commander::EXPANSION_INTERVAL_MIN)
	{
		if (mSelfClassifier->isAttacking()) {
			issueCommand(Command_Expand, false, Reason_BotAttacking);
		} else if (mSelfClassifier->areExpansionsSaturated()) {
			issueCommand(Command_Expand, false, Reason_BotExpansionRunningLow);
		} else if (mSelfClassifier->isAnExpansionLowOnMinerals()) {
			issueCommand(Command_Expand, false, Reason_BotExpansionsSaturated);
		}
	}


	// ATTACK
	// -> When expanding, when upgrade soon is done
	/// @todo when shall we drop and when shall we attack?
	if (!mSelfClassifier->isAttacking()) {
		if (mSelfClassifier->isExpanding()) {
			issueCommand(Command_Attack, false, Reason_BotExpanding);
		} else {
			vector<UnitAgent*> freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free);
			if (mSelfClassifier->isUpgradeSoonDone(freeUnits)) {
				issueCommand(Command_Attack, false, Reason_BotUpgradeSoonDone);
			}
		}
	}


	// SCOUT
	// Always try to have a scout out after we have x workers
	
}

void Commander::computeAlliedReactions() {
	if (!config::module::ALLIED_REACT) {
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
			issueCommand(Command_Attack, false, Reason_AlliedMovingToAttack);
		}
	}

	// Small is active -> Create distraction if we don't have a distraction out
	if (smallActive) {
		const vector<AttackSquadPtr>& distractingAttacks = mSquadManager->getDistractingAttacks();

		if (distractingAttacks.empty()) {
			issueCommand(Command_Drop, false, Reason_AlliedMovingToAttack);
		}
	}


	/// @todo check for player expanding
}

void Commander::issueCommand(const std::string& command) {
	if (isCommandAvailable(command)) {
		issueCommand(mCommandStringToEnums[command], true);
	}
}

void Commander::issueCommand(Commands command, bool alliedOrdered, Reasons reason) {
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
		orderAttack(alliedOrdered, reason);
		break;

	case Command_Drop:
		orderDrop(alliedOrdered, reason);
		break;

	case Command_Expand:
		orderExpand(alliedOrdered, reason);
		break;

	case Command_Scout:
		orderScout(alliedOrdered, reason);
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

void Commander::orderAttack(bool alliedOrdered, Reasons reason) {
	// Never do a frontal attack if we're under attack (defending)
	if (mDefenseManager->isUnderAttack()) {
		if (alliedOrdered) {
			mIntentionWriter->writeIntention(Intention_BotAttackNot, Reason_BotIsUnderAttack);
		}
		return;
	}

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
			if (alliedOrdered) {
				mIntentionWriter->writeIntention(Intention_BotAttackMerged);
			}
		} else {
			AttackSquad* attackSquad = new AttackSquad(freeUnits);

			if (attackSquad->isFollowingAlliedSquad()) {
				mIntentionWriter->writeIntention(Intention_AlliedAttackFollow, reason);
			} else {
				const TilePosition attackPos = attackSquad->getGoalPosition();
				mIntentionWriter->writeIntention(Intention_BotAttack, reason, attackPos);
			}

		}
	} else {
		if (alliedOrdered) {
			mIntentionWriter->writeIntention(Intention_BotAttackNot, Reason_BotNotEnoughUnits);
		}
	}
}

void Commander::orderDrop(bool alliedOrdered, Reasons reason) {
	/// @todo Check how many attacks we have (frontal, distracting)
	/// Can we create a distracting attack?

	/// @todo get defensive squad units for a counter-attack drop.

	// Get available unit compositions
	vector<UnitAgent*> freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free | UnitFilter_WorkersFree);
	vector<UnitComposition> availableUnitCompositions;
	availableUnitCompositions = mUnitCompositionFactory->getUnitCompositionsByType(freeUnits, UnitComposition_Drop);


	// Create drop
	if (!availableUnitCompositions.empty()) {
		// Choose a random unit composition
		unsigned int randomId = rand() % availableUnitCompositions.size();
		const UnitComposition& chosenComposition = availableUnitCompositions[randomId];

		DropSquad* dropSquad = new DropSquad(freeUnits, chosenComposition);

		mIntentionWriter->writeIntention(Intention_BotDrop, reason, dropSquad->getGoalPosition());
	}
	// No drops available
	else {
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Commander::createDrop() | No drops available");

		if (alliedOrdered) {
			mIntentionWriter->writeIntention(Intention_BotDropNot, Reason_BotNotEnoughUnits);
		}
	}
}

void Commander::orderScout(bool alliedOrdered, Reasons reason) {
	/// @todo what about existing scout, remove it?
	
	// Get available unit compositions
	//std::vector<UnitAgent*> freeUnits = munitManager->getUnitsByFilter(UnitFilter_WorkersFree);
	//UnitAgent* unit = freeUnits.at(0);
	//freeUnits.clear();
	//freeUnits.push_back(unit);
	//ScoutSquad* pScoutSuad = new ScoutSquad(freeUnits);
	//mSquadWaiting = pScoutSuad->getThis();
	
	// Return all free units (including from defensive squads) and all workers that are free (is neither building nor in a squad)
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
		mIntentionWriter->writeIntention(Intention_BotScout, reason);
	} else {
		if (alliedOrdered) {
			mIntentionWriter->writeIntention(Intention_BotScoutNot, Reason_BotNotEnoughUnits);
		}
	}
}

void Commander::orderExpand(bool alliedOrdered, Reasons reason) {
	if(BuildPlanner::getInstance()->isExpansionAvailable()) {
		BuildPlanner::getInstance()->expand();
		mIntentionWriter->writeIntention(Intention_BotExpand, reason);
		mExpansionTimeLast;
	} else {
		DEBUG_MESSAGE(utilities::LogLevel_Info, "Expansion not available yet");
		/// @todo write the reason why we can't expand
		if (alliedOrdered) {
			mIntentionWriter->writeIntention(Intention_BotExpandNot);
		}
	}
}

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