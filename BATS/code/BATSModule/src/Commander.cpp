#include "Commander.h"
#include "UnitManager.h"
#include "Squad.h"
#include "SquadManager.h"
#include "PlayerArmyManager.h"
#include "AlliedSquad.h"
#include "UnitManager.h"
#include "IntentionWriter.h"
#include "UnitCompositionFactory.h"
#include "SelfClassifier.h"
#include "AlliedClassifier.h"
#include "AttackSquad.h"
#include "DropSquad.h"
#include "ScoutSquad.h"
#include "BuildPlanner.h"
#include "GameTime.h"
#include "BuildPlanner.h"

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
	mIntentionWriter = NULL;
	mSelfClassifier = NULL;
	mGameTime = NULL;
	mBuildPlanner = NULL;
	mAlliedClassifier = NULL;

	mAlliedArmyManager = PlayerArmyManager::getInstance();
	mSquadManager = SquadManager::getInstance();
	mUnitManager = UnitManager::getInstance();
	mUnitCompositionFactory = UnitCompositionFactory::getInstance();
	mIntentionWriter = IntentionWriter::getInstance();
	mSelfClassifier = SelfClassifier::getInstance();
	mGameTime = GameTime::getInstance();
	mBuildPlanner = BuildPlanner::getInstance();
	mAlliedClassifier = AlliedClassifier::getInstance();

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
	if (!mSelfClassifier->isUnderAttack() &&
		!mSelfClassifier->isExpanding() &&
		mSelfClassifier->getActiveExpansionCount() < config::commander::EXPANSION_ACTIVE_MAX &&
		mSelfClassifier->getLastExpansionStartTime() > config::commander::EXPANSION_INTERVAL_MIN)
	{
		if (mSelfClassifier->isAttacking()) {
			issueCommand(Command_Expand, false, Reason_BotAttacking);
		} else if (mSelfClassifier->areExpansionsSaturated()) {
			issueCommand(Command_Expand, false, Reason_BotExpansionsSaturated);
		} else if (mSelfClassifier->isAnExpansionLowOnMinerals()) {
			issueCommand(Command_Expand, false, Reason_BotExpansionRunningLow);
		} else if (mSelfClassifier->isHighOnMinerals()) {
			issueCommand(Command_Expand, false, Reason_BotHighOnResources);
		}
	}


	// ATTACK
	// -> When expanding, when upgrade soon is done
	/// @todo when shall we drop and when shall we attack?
	if (!mSelfClassifier->isUnderAttack() && !mSelfClassifier->isAttacking()) {
		if (mSelfClassifier->isExpanding()) {
			issueCommand(Command_Attack, false, Reason_BotExpanding);
		} else {
			const vector<UnitAgent*>& freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free);
			if (mSelfClassifier->isUpgradeSoonDone(freeUnits)) {
				issueCommand(Command_Attack, false, Reason_BotUpgradeSoonDone);
			}
		}
	}


	// SCOUT
	// -> Always after we have x workers, but not under attack
	if (!mSelfClassifier->isUnderAttack() && !mSelfClassifier->isScouting()) {
		int cWorkers = mUnitManager->getWorkerCount();

		if (cWorkers >= config::commander::SCOUT_ON_WORKER_COUNT) {
			issueCommand(Command_Scout, false);
		}
	}


	// TRANSITION
	// -> When we're high on resources and no buildings are in the build order
	if (mBuildPlanner->canTransition() &&
		mBuildPlanner->getQueueCount() == 0 &&
		mSelfClassifier->isHighOnResources())
	{
		issueCommand(Command_Transition, false, Reason_BotHighOnResources);
	}
}

void Commander::computeAlliedReactions() {
	if (!config::module::ALLIED_REACT) {
		return;
	}


	// ALLIED ATTACKING
	// Check for active allied squads
	vector<AlliedSquadCstPtr> squads = mAlliedArmyManager->getSquads<AlliedSquad>();
	bool frontalActive = false;
	bool distractingActive = false;
	for (size_t i = 0; i < squads.size(); ++i) {
		if (squads[i]->isActive()) {
			// Big
			if (squads[i]->isFrontalAttack()) {
				frontalActive = true;
			} else {
				distractingActive = true;
			}
		}
	}

	// Big is active -> Attack with frontal, then a drop?
	if (frontalActive) {
		if (!mSelfClassifier->hasFrontalAttack() && mSelfClassifier->canFrontalAttack()) {
			issueCommand(Command_Join, false, Reason_AlliedMovingToAttack);
		} else if (!mSelfClassifier->hasDrop()) {
			issueCommand(Command_Drop, false, Reason_AlliedMovingToAttack);
		}
	}

	// Small is active -> Create distraction if we don't have a distraction out
	if (distractingActive && !mSelfClassifier->hasDrop()) {
		issueCommand(Command_Drop, false, Reason_AlliedMovingToAttack);
	}



	// ALLIED EXPANDING
	// Attack with a drop first, if cannot, try frontal attack
	if (mAlliedClassifier->isExpanding() &&
		!mSelfClassifier->isUnderAttack() && 
		!mSelfClassifier->isAttacking())
	{
		if (mSelfClassifier->canDrop()) {
			issueCommand(Command_Drop, false, Reason_AlliedExpanding);
			issueCommand(Command_Drop, false);
		} else {
			issueCommand(Command_Attack, false, Reason_AlliedExpanding);
		}
	}
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

	case Command_Join:
		orderJoin(alliedOrdered, reason);
		break;

	case Command_Scout:
		orderScout(alliedOrdered, reason);
		break;

	case Command_Transition:
		orderTransition(alliedOrdered, reason);
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
	if (mSelfClassifier->isUnderAttack()) {
		if (alliedOrdered) {
			mIntentionWriter->writeIntention(Intention_BotAttackNot, Reason_BotIsUnderAttack);
		}
		return;
	}


	// Get free units
	std::vector<UnitAgent*> freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free);

	bool canAttack;
	// Allied ordered Only add if we have free units
	if (alliedOrdered) {
		canAttack = !freeUnits.empty();
	} else {
		canAttack = mSelfClassifier->canFrontalAttack(freeUnits);
	}

	if (canAttack) {
		// Add the units to the old attack squad if it exists
		AttackSquadPtr oldSquad = mSquadManager->getFrontalAttack();

		if (oldSquad != NULL) {
			oldSquad->addUnits(freeUnits);
			mIntentionWriter->writeIntention(Intention_BotAttackMerged, reason);
		} else {
			AttackSquad* attackSquad = new AttackSquad(freeUnits);
			const TilePosition attackPos = attackSquad->getGoalPosition();
			mIntentionWriter->writeIntention(Intention_BotAttack, reason, attackPos);
		}
	} else {
		if (alliedOrdered) {
			mIntentionWriter->writeIntention(Intention_BotAttackNot, Reason_BotNotEnoughUnits);
		}
	}
}

void Commander::orderJoin(bool alliedOrdered, Reasons reason) {
	bool orderSuccess = false;
	Intentions sendIntention = Intention_Lim;
	Reasons sendReason = reason;

	// Do we have an existing squad and it's not following -> Join with it
	AttackSquadPtr attackSquad = mSquadManager->getFrontalAttack();

	if (NULL != attackSquad && !attackSquad->isFollowingAlliedSquad()) {
		sendIntention = Intention_AlliedAttackFollowNot;

		AlliedSquadCstPtr alliedSquad = mAlliedArmyManager->getAlliedFrontalSquad();
		
		if (NULL != alliedSquad) {
			attackSquad->followAlliedSquad(alliedSquad);
			sendIntention = Intention_AlliedAttackFollow;
			orderSuccess = true;
		}
		// There's no allied squad to follow
		else {
			sendReason = Reason_AlliedAttackNotExist;
		}
	}


	// Else if, existing squad and following -> Add more units to the mix
	else if (NULL != attackSquad && attackSquad->isFollowingAlliedSquad()) {
		sendIntention = Intention_BotAttackMergedNot;

		// Only add units to the attack if we're not under attack
		if (!mSelfClassifier->isUnderAttack()) {
			const std::vector<UnitAgent*>& freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free);

			if (!freeUnits.empty()) {
				attackSquad->addUnits(freeUnits);
				sendIntention = Intention_BotAttackMerged;
				orderSuccess = true;
			} else {
				sendReason = Reason_BotNotEnoughUnits;
			}
		} else {
			sendReason = Reason_BotIsUnderAttack;
		}
	}


	// Else -> Create a new squad and follow
	else {
		sendIntention = Intention_AlliedAttackFollowNot;

		// Only add units to the attack if we're not under attack
		if (!mSelfClassifier->isUnderAttack()) {
			const std::vector<UnitAgent*>& freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_Free);

			bool canAttack;
			if (alliedOrdered) {
				canAttack = !freeUnits.empty();
			} else {
				canAttack = mSelfClassifier->canFrontalAttack(freeUnits);
			}

			if (canAttack) {
				AlliedSquadCstPtr alliedSquad = mAlliedArmyManager->getAlliedFrontalSquad();

				if (NULL != alliedSquad) {
					new AttackSquad(freeUnits, false, UnitCompositionFactory::INVALID, alliedSquad);
					sendIntention = Intention_AlliedAttackFollow;
					orderSuccess = true;
				} else {
					sendReason = Reason_AlliedAttackNotExist;
				}
			} else {
				sendReason = Reason_BotNotEnoughUnits;
			}
		} else {
			sendReason = Reason_BotIsUnderAttack;
		}
	}

	if (orderSuccess) {
		mIntentionWriter->writeIntention(sendIntention, sendReason);
	} else if (alliedOrdered) {
		mIntentionWriter->writeIntention(sendIntention, sendReason);
	}
}

void Commander::orderDrop(bool alliedOrdered, Reasons reason) {
	/// @todo Check how many attacks we have (frontal, distracting)
	/// Can we create a distracting attack?

	/// @todo get defensive squad units for a counter-attack drop, even when we're under attack

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

void Commander::orderTransition(bool alliedOrdered, Reasons reason) {

	// Only transition if we're not in late game already
	if (mBuildPlanner->canTransition()) {
		Intentions intention = Intention_Lim;
		if (mBuildPlanner->getCurrentPhase() == "early") {
			intention = Intention_BotTransitionMid;
		} else if (mBuildPlanner->getCurrentPhase() == "mid") {
			intention = Intention_BotTransitionLate;
		}
		mIntentionWriter->writeIntention(intention, reason);
		
		mBuildPlanner->switchToPhase("");
	} else {
		if (alliedOrdered) {
			mIntentionWriter->writeIntention(Intention_BotTransitionNot, Reason_BotTransitionNoMore);
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
	size_t duplicates = 0;

	mCommandStringToEnums["attack"] = Command_Attack;
	mCommandStringToEnums["drop"] = Command_Drop;
	mCommandStringToEnums["expand"] = Command_Expand;
	mCommandStringToEnums["join"] = Command_Join;
	mCommandStringToEnums["follow"] = Command_Join; // Follow is synonym to join
	duplicates++;
	mCommandStringToEnums["scout"] = Command_Scout;
	mCommandStringToEnums["transition"] = Command_Transition;

	if (mCommandStringToEnums.size() != Command_Lim + duplicates) {
		ERROR_MESSAGE(false, "Commander: Command String to enum does not have same size as enumerations!");
	}
}

bool Commander::isAlliedCreatingCommand() const {
	return NULL != mAlliedSquadCommand;
}