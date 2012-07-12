#include "AttackSquad.h"
#include "AttackCoordinator.h"
#include "Config.h"
#include "ExplorationManager.h"
#include "AlliedSquad.h"
#include "AlliedArmyManager.h"
#include "Helper.h"
#include <sstream>
#include <iomanip>

using namespace bats;
using namespace BWAPI;
using namespace std::tr1;
using namespace std;

AttackCoordinator* AttackSquad::mpsAttackCoordinator = NULL;
ExplorationManager* AttackSquad::mpsExplorationManager = NULL;
AlliedArmyManager* AttackSquad::mpsAlliedArmyManager = NULL;

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

	if (mpsAttackCoordinator == NULL) {
		mpsAttackCoordinator = AttackCoordinator::getInstance();
		mpsExplorationManager = ExplorationManager::getInstance();
		mpsAlliedArmyManager = AlliedArmyManager::getInstance();
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
	// Not following any allied squad
	if (NULL == mpAlliedSquadFollow) {
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


	// Else - Following an allied squad
	else {
		// Allied squad might have merged, search for a close squad (that's not home)
		if(mpAlliedSquadFollow->isEmpty()) {
			mpAlliedSquadFollow.reset();

			vector<pair<AlliedSquadCstPtr, int>> foundSquads;
			foundSquads = mpsAlliedArmyManager->getSquadsWithin(
				getCenter(),
				config::squad::attack::FIND_ALLIED_SQUAD_DISTANCE,
				true
			);

			// Chose the closest squad that is outside home (not idle)
			vector<pair<AlliedSquadCstPtr, int>>::const_iterator squadIt = foundSquads.begin();
			while (NULL == mpAlliedSquadFollow && squadIt != foundSquads.end()) {
				if (squadIt->first->getState() != AlliedSquad::State_Idle) {
					mpAlliedSquadFollow = squadIt->first;
				}

				++squadIt;
			}
		}

		// Follow the allied squad
		if (NULL != mpAlliedSquadFollow) {

			switch (mpAlliedSquadFollow->getState()) {
				// Allied are still -> do nothing
				case AlliedSquad::State_AttackHalted:
					handleAlliedRegrouping();
					setAvoidEnemyUnits(true);
					break;
				
				// Allied is retreating -> go to target position, don't attack
				case AlliedSquad::State_Retreating: {
					if (isRegroupingWithAllied()) {
						clearAlliedRegrouping();
					}
					TilePosition targetPosition = mpAlliedSquadFollow->getTargetPosition();
					// Only update position if it changed
					if (getGoalPosition() != targetPosition) {
						setGoalPosition(mpAlliedSquadFollow->getTargetPosition());
					}
					setAvoidEnemyUnits(true);
					break;
				}

				// Allied is moving	-> go to target position, attack if see anything.
				case AlliedSquad::State_MovingToAttack: {
					if (isRegroupingWithAllied()) {
						clearAlliedRegrouping();
					}
					TilePosition targetPosition = mpAlliedSquadFollow->getTargetPosition();
					// Only update position if it changed
					if (getGoalPosition() != targetPosition) {
						setGoalPosition(mpAlliedSquadFollow->getTargetPosition());
					}
					setAvoidEnemyUnits(false);
					break;
				}

				// Allied is attacking -> Find something close to attack
				case AlliedSquad::State_Attacking:
					/// @todo don't call request attack every frame!
					handleAlliedRegrouping();
					if (!isRegroupingWithAllied()) {
						setAvoidEnemyUnits(false);
						mpsAttackCoordinator->requestAttack(getThis());
					}
					break;

				/// @todo Allied is safe -> disband (change to retreat)
				case AlliedSquad::State_Idle:
					forceDisband();
					mpAlliedSquadFollow.reset();
					break;
			}
		}
		// @todo Did not find an allied squad, disband (change to retreat)
		else {
			forceDisband();
		}
	}
}

bool AttackSquad::createGoal() {
	// Check if allied big frontal attack is out of home
	if (NULL == mpAlliedSquadFollow && !isDistracting()) {
		AlliedSquadCstPtr pBigAlliedSquad = mpsAlliedArmyManager->getBigSquad();

		// Check if squad is outside of home
		if (NULL != pBigAlliedSquad && !pBigAlliedSquad->isEmpty()) {
			if (pBigAlliedSquad->getState() != AlliedSquad::State_Idle) {
				mpAlliedSquadFollow = pBigAlliedSquad;
				setCanRegroup(false);
			}
		}
	}


	// Not following allied -> request regular attack
	if (NULL == mpAlliedSquadFollow) {
		mpsAttackCoordinator->requestAttack(getThis());
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
		!mpsExplorationManager->hasSpottedBuildingWithinRange(getGoalPosition(), config::squad::attack::STRUCTURES_DESTROYED_GOAL_DISTANCE))
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
	const std::vector<UnitAgent*> units = getUnits();

	for (size_t i = 0; i < units.size(); ++i) {
		BWAPI::Unit* currentUnit = units[i]->getUnit();
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

	if (getSquaredDistance(getCenter(), mpAlliedSquadFollow->getCenter())
		>=
		config::squad::attack::ALLIED_REGROUP_BEGIN_SQUARED)
	{
		return true;
	} else {
		return false;
	}
}

bool AttackSquad::finishedAlliedRegrouping() const {
	if (NULL != mpAlliedSquadFollow || mpAlliedSquadFollow->getCenter() == TilePositions::Invalid) {
		return true;
	}

	if (getSquaredDistance(getCenter(), mpAlliedSquadFollow->getCenter())
		<=
		config::squad::attack::ALLIED_REGROUP_END_SQUARED)
	{
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
	const vector<UnitAgent*>& units = getUnits();

	for (size_t i = 0; i < units.size(); ++i) {
		Unit* pUnit = units[i]->getUnit();
		Unit* pTarget = pUnit->getTarget();
		Unit* pOrderTarget = pUnit->getOrderTarget();

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
	} else {
		/// @todo retreat to home instead of just disbanding
		forceDisband();
	}
}