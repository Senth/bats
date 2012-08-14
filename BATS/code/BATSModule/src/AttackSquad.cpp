#include "AttackSquad.h"
#include "AttackCoordinator.h"
#include "Config.h"
#include "ExplorationManager.h"
#include "AlliedSquad.h"
#include "EnemySquad.h"
#include "PlayerArmyManager.h"
#include "Helper.h"
#include "SquadDefs.h"
#include "DefenseManager.h"
#include "IntentionWriter.h"
#include <sstream>
#include <iomanip>

using namespace bats;
using namespace BWAPI;
using namespace std::tr1;
using namespace std;

AttackCoordinator* AttackSquad::msAttackCoordinator = NULL;
ExplorationManager* AttackSquad::msExplorationManager = NULL;
PlayerArmyManager* AttackSquad::msPlayerArmyManager = NULL;
DefenseManager* AttackSquad::msDefenseManager = NULL;

const std::string ATTACK_SQUAD_NAME = "AttackSquad";

AttackSquad::AttackSquad(
	const std::vector<UnitAgent*>& units,
	bool distracting,
	const UnitComposition& unitComposition)
	:
	Squad(units, distracting, true, unitComposition)
{
	mDistraction = distracting;
	mWaitInPosition = false;
	mAttackedEnemyStructures = false;

	if (msAttackCoordinator == NULL) {
		msAttackCoordinator = AttackCoordinator::getInstance();
		msExplorationManager = ExplorationManager::getInstance();
		msPlayerArmyManager = PlayerArmyManager::getInstance();
		msDefenseManager = DefenseManager::getInstance();
	}
}

AttackSquad::~AttackSquad() {
	// Does nothing
}

void AttackSquad::handleAlliedRegrouping() {
	if (NULL == mpAlliedSquadFollow) {
		return;
	}

	// Regroup with allied force if far away
	if (getTemporaryGoalPosition() == TilePositions::Invalid) {
		if (needsAlliedRegrouping()) {
			setAvoidEnemyUnits(true);
			setTemporaryGoalPosition(mpAlliedSquadFollow->getCenter());
		}
	}
	// Currently regrouping, update regroup position if not finished
	else {
		if (finishedAlliedRegrouping()) {
			clearAlliedRegrouping();
		} else {
			setTemporaryGoalPosition(mpAlliedSquadFollow->getCenter());
		}
	}
}

bool AttackSquad::isRegroupingWithAllied() const {
	return getTemporaryGoalPosition().isValid();
}

void AttackSquad::clearAlliedRegrouping() {
	setTemporaryGoalPosition(TilePositions::Invalid);
}

void AttackSquad::updateDerived() {
	handleNormalBehavior();
	handleFollowAllied();
	handleRetreat();
}

void AttackSquad::handleNormalBehavior() {
	// Early return if following an allied squad
	if (NULL != mpAlliedSquadFollow) {
		return;
	}


	// Never regroup if attacking
	if (isAttacking()) {
		setCanRegroup(false);
	} else {
		setCanRegroup(true);
	}


	// Check if we're attacking an enemy structure.
	if (!mAttackedEnemyStructures && isTargetingEnemyStructure()) {
		mAttackedEnemyStructures = true;
	}


	// Create a wait position when we're in position, if we shall wait that is.
	if (mWaitInPosition && isInPosition()) {
		if (hasTemporaryGoalPosition()) {
			// All wait goals done, continue to goal
			if (!hasWaitGoals()) {
				mWaitInPosition = false;
				setTemporaryGoalPosition(BWAPI::TilePositions::Invalid);
			}
		} else {
			setTemporaryGoalPosition(getCenter());
		} 
	}
}

void AttackSquad::handleFollowAllied() {
	// Early return if not following an allied squad
	if (NULL == mpAlliedSquadFollow) {
		return;
	}


	// Allied squad might have merged, search for a close squad (that's not home)
	if(mpAlliedSquadFollow->isEmpty()) {
		mpAlliedSquadFollow.reset();

		vector<pair<AlliedSquadCstPtr, int>> foundSquads =
			msPlayerArmyManager->getSquadsWithin<AlliedSquad>(
			getCenter(),
			config::squad::attack::FIND_ALLIED_SQUAD_DISTANCE,
			true
			);

		// Chose the closest squad that is outside home (not idle)
		vector<pair<AlliedSquadCstPtr, int>>::const_iterator squadIt = foundSquads.begin();
		while (NULL == mpAlliedSquadFollow && squadIt != foundSquads.end()) {
			if (squadIt->first->getState() != AlliedSquad::State_IdleInBase) {
				mpAlliedSquadFollow = squadIt->first;
			}

			++squadIt;
		}
	}

	// Follow the allied squad
	if (NULL != mpAlliedSquadFollow) {

		switch (mpAlliedSquadFollow->getState()) {
			// Allied is still -> do nothing
			case AlliedSquad::State_IdleOutsideBase:
				handleAlliedRegrouping();
				setAvoidEnemyUnits(true);
				break;

			// Allied is retreating -> go to allied target position, don't attack
			case AlliedSquad::State_Retreating: {
				if (isRegroupingWithAllied()) {
					clearAlliedRegrouping();
				}
				setGoalPosition(mpAlliedSquadFollow->getTargetPosition());
				setAvoidEnemyUnits(true);
				break;
			}

			// Allied is moving	-> go to target position, attack if see anything.
			case AlliedSquad::State_MovingToAttack: {
				if (isRegroupingWithAllied()) {
					clearAlliedRegrouping();
				}
				setGoalPosition(mpAlliedSquadFollow->getTargetPosition());
				setAvoidEnemyUnits(false);
				break;
			}

			// Allied is attacking -> Find something close to attack
			case AlliedSquad::State_Attacking:
				/// @todo don't call request attack every frame!
				handleAlliedRegrouping();
				if (!isRegroupingWithAllied()) {
					setAvoidEnemyUnits(false);
					msAttackCoordinator->requestAttack(getThis());
				}
				break;

			// Allied is safe -> retreat, then merge with Patrol Squad (disband)
			case AlliedSquad::State_IdleInBase:
				setRetreatPosition(msDefenseManager->findRetreatPosition());
				mpAlliedSquadFollow.reset();
				break;
		}
	}
	// Did not find an allied squad -> retreat, then merge with Patrol Squad (disband)
	else {
		setRetreatPosition(msDefenseManager->findRetreatPosition());
	}
}

void AttackSquad::handleRetreat() {
	// Skip i squad is empty
	if (isEmpty()) {
		return;
	}


	EnemySquadCstPtr pEnemy = msPlayerArmyManager->getClosestSquad<EnemySquad>(
		getCenter(),
		config::classification::retreat::ENEMY_CLOSE_DISTANCE).first;

	// Skip if no enemy is present
	if (NULL == pEnemy) {
		return;
	}


	bool retreat = false;

	// Check if enemy army is larger than our size (probably a lot larger)
	int cEnemySupplies = pEnemy->getSupplyCount();
	int cOurSupplies = getSupplyCount();
	if (NULL != mpAlliedSquadFollow) {
		cOurSupplies += mpAlliedSquadFollow->getSupplyCount();
	}

	if (cOurSupplies * config::classification::retreat::ENEMY_LARGER_THAN_US <= cEnemySupplies) {
		retreat = true;
	}


	// Check if our supplies are decreasing too quickly
	int cEnemyDeltaSupplies = pEnemy->getDeltaSupplyCount();
	int cOurDeltaSupplies = getDeltaSupplyCount();
	/// @todo Don't use allied squads for now, if they split this can cause great inaccuracies
	//if (NULL != mpAlliedSquadFollow) {
	//	cOurDeltaSupplies += mpAlliedSquadFollow->getDeltaSupplyCount();
	//}

	// If our supplies decrease very fast and the enemy's is not -> retreat
	if (cOurDeltaSupplies < -config::classification::retreat::SUPPLY_DECREASING_FAST &&
		cEnemyDeltaSupplies > -config::classification::retreat::SUPPLY_DECREASING_FAST)
	{
		retreat = true;
	}


	/// @todo Check if enemy has choke point, then we shall probably not engage
	/// For now this is solved when our units decrease very fast, but not the enemy's,
	/// but we would like to not loose a lot of our units before we can deduct that this
	/// is a bad engagement.


	// Handle retreat
	if (retreat) {
		if (isFollowingAlliedSquad()) {
			msIntentionWriter->writeIntention(Intention_WeShouldRetreat, Reason_EnemyTooStrong);
		}
		else {
			setRetreatPosition(msDefenseManager->findRetreatPosition());
			msIntentionWriter->writeIntention(Intention_BotRetreat, Reason_EnemyTooStrong);
			
		}
	}
}

bool AttackSquad::createGoal() {
	// Check if allied big frontal attack is out of home
	if (NULL == mpAlliedSquadFollow && isFrontalAttack()) {
		AlliedSquadCstPtr pBigAlliedSquad = msPlayerArmyManager->getAlliedFrontalSquad();

		// Check if squad is outside of home
		if (NULL != pBigAlliedSquad && !pBigAlliedSquad->isEmpty()) {
			if (pBigAlliedSquad->getState() != AlliedSquad::State_IdleInBase) {
				mpAlliedSquadFollow = pBigAlliedSquad;
				setCanRegroup(false);
			}
		}
	}


	// Not following allied -> request regular attack
	if (NULL == mpAlliedSquadFollow) {
		msAttackCoordinator->requestAttack(getThis());
	}

	return true;
}

Squad::GoalStates AttackSquad::checkGoalState() const {
	Squad::GoalStates goalState = Squad::GoalState_NotCompleted;

	// Only check goal if we're not following an allied squad
	if (NULL == mpAlliedSquadFollow) {
		// Check if all enemy structures are dead nearby, and we have attacked one
		if (isEnemyStructuresNearGoalDead()) {
			goalState = Squad::GoalState_Succeeded;
		}

		/// @todo check if enemy force is too strong.

		/// @todo check for enemy units to kill them?
	}

	return goalState;
}

bool AttackSquad::isEnemyStructuresNearGoalDead() const {
	/// @todo check the whole radius for buildings, some structures might be hidden from the view
	/// all will therefore not be seen (and thus the goal is completed when it should not be).
	if (Broodwar->isVisible(getGoalPosition()) &&
		!msExplorationManager->hasSpottedBuildingWithinRange(getGoalPosition(), config::squad::attack::STRUCTURES_DESTROYED_GOAL_DISTANCE))
	{
		return true;
	} else {
		return false;
	}
}

bool AttackSquad::isInPosition() const {
	return isCloseTo(getGoalPosition(), config::squad::attack::WAITING_POSITION_DISTANCE_FROM_GOAL);
}

bool AttackSquad::isAttacking() const {
	const std::vector<const UnitAgent*> units = getUnits();

	for (size_t i = 0; i < units.size(); ++i) {
		const BWAPI::Unit* currentUnit = units[i]->getUnit();
		if (currentUnit->isAttacking() || currentUnit->isUnderAttack()) {
			return true;
		}
	}

	return false;
}

bool AttackSquad::isReadyToAttack() const {
	return isInPosition() || isAttacking();
}

bool AttackSquad::isDistracting() const {
	return mDistraction;
}

bool AttackSquad::isFrontalAttack() const {
	return !mDistraction;
}

bool AttackSquad::isFollowingAlliedSquad() const {
	return NULL != mpAlliedSquadFollow;
}

AlliedSquadCstPtr AttackSquad::getAlliedSquad() const {
	return mpAlliedSquadFollow;
}

std::string AttackSquad::getName() const {
	return ATTACK_SQUAD_NAME;
}

AttackSquadPtr AttackSquad::getThis() const {
	return static_pointer_cast<AttackSquad>(Squad::getThis());
}

#pragma warning(push)
#pragma warning(disable:4100)
void AttackSquad::onWaitGoalAdded(const std::tr1::shared_ptr<WaitGoal>& newWaitGoal) {
	mWaitInPosition = true;
}
#pragma warning(pop)

bool AttackSquad::needsAlliedRegrouping() const {
	if (NULL == mpAlliedSquadFollow || mpAlliedSquadFollow->getCenter() == TilePositions::Invalid) {
		return false;
	}

	if (!isCloseTo(mpAlliedSquadFollow->getCenter(), config::squad::attack::ALLIED_REGROUP_BEGIN)) {
		return true;
	} else {
		return false;
	}
}

bool AttackSquad::finishedAlliedRegrouping() const {
	if (NULL != mpAlliedSquadFollow || mpAlliedSquadFollow->getCenter() == TilePositions::Invalid) {
		return true;
	}

	if (isCloseTo(mpAlliedSquadFollow->getCenter(), config::squad::attack::ALLIED_REGROUP_END)) {
		return true;
	} else {
		return false;
	}
}

string AttackSquad::getDebugInfo() const {
	stringstream ss;
	ss << left << setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Follow: ";

	if (NULL != mpAlliedSquadFollow) {
		ss << mpAlliedSquadFollow->getId();
	} else {
		ss << "None";
	}

	ss << "\n";

	return Squad::getDebugInfo() + ss.str();
}

bool AttackSquad::isTargetingEnemyStructure() const {
	const vector<const UnitAgent*>& units = getUnits();

	for (size_t i = 0; i < units.size(); ++i) {
		const Unit* unit = units[i]->getUnit();
		const Unit* pTarget = unit->getTarget();
		const Unit* pOrderTarget = unit->getOrderTarget();

		// Target
		if (NULL != pTarget &&
			Broodwar->self()->isEnemy(pTarget->getPlayer()) &&
			pTarget->getType().isBuilding())
		{
			return true;
		}
		// Order target
		else if (NULL != pOrderTarget &&
			Broodwar->self()->isEnemy(pOrderTarget->getPlayer()) &&
			pOrderTarget->getType().isBuilding())
		{
			return true;
		}
	}

	return false;
}

void AttackSquad::onGoalSucceeded() {
	// If we succeeded without actually attacking anything, try another goal
	if (!mAttackedEnemyStructures) {
		DEBUG_MESSAGE(utilities::LogLevel_Info, "AttackSquad never attacked, finding another goal...");
		clearMovement();
		createGoal();
		msIntentionWriter->writeIntention(Intention_BotAttackNewPosition, Reason_BotDidNotAttack, getGoalPosition());
	} else {
		// Retreat
		msIntentionWriter->writeIntention(Intention_BotRetreat, Reason_BotAttackSuccess);
		setRetreatPosition(msDefenseManager->findRetreatPosition());
	}
}