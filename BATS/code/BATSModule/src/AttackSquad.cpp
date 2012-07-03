#include "AttackSquad.h"
#include "AttackCoordinator.h"
#include "Config.h"
#include "ExplorationManager.h"
#include "AlliedSquad.h"
#include "AlliedArmyManager.h"
#include "Helper.h"

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

	if (mpsAttackCoordinator == NULL) {
		mpsAttackCoordinator = AttackCoordinator::getInstance();
		mpsExplorationManager = ExplorationManager::getInstance();
		mpsAlliedArmyManager = AlliedArmyManager::getInstance();
	}
}

AttackSquad::~AttackSquad() {
	// Does nothing
}

void AttackSquad::computeSquadSpecificActions() {
	// Not following any allied squad
	if (NULL == mpAlliedSquadFollow) {
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
			while (NULL != mpAlliedSquadFollow && squadIt != foundSquads.end()) {
				if (squadIt->first->getState() != AlliedSquad::State_Idle) {
					mpAlliedSquadFollow = squadIt->first;
				}

				++squadIt;
			}
		}

		// Follow the allied squad
		if (NULL != mpAlliedSquadFollow) {
			// Regroup with allied force if far away
			if (getTemporaryGoalPosition() == TilePositions::Invalid) {
				if (needsAlliedRegrouping()) {
					setTemporaryGoalPosition(mpAlliedSquadFollow->getCenter());
					setAvoidEnemyUnits(true);
				}
			}
			// Currently regrouping, update regroup position if not finished
			else {
				if (finishedAlliedRegrouping()) {
					setTemporaryGoalPosition(TilePositions::Invalid);
					setAvoidEnemyUnits(false);
				} else {
					setTemporaryGoalPosition(mpAlliedSquadFollow->getCenter());
				}
			}

			// No regrouping
			if (getTemporaryGoalPosition() == TilePositions::Invalid) {
				switch (mpAlliedSquadFollow->getState()) {
					// Allied are still -> do nothing
					case AlliedSquad::State_AttackHalted:
						setAvoidEnemyUnits(true);
						break;
				
					// Allied is retreating ->
					// Allied is moving		-> follow, but don't attack
					case AlliedSquad::State_Retreating:
					case AlliedSquad::State_MovingToAttack:
						setGoalPosition(mpAlliedSquadFollow->getCenter());
						setAvoidEnemyUnits(true);
						break;

					// Allied is attacking -> Find something close to attack
					case AlliedSquad::State_Attacking:
						/// @todo don't call request attack every frame!
						setAvoidEnemyUnits(false);
						mpsAttackCoordinator->requestAttack(getThis());
						break;

					/// @todo Allied is safe -> disband (change to retreat)
					case AlliedSquad::State_Idle:
						forceDisband();
						mpAlliedSquadFollow.reset();
						break;
				}
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
		// Check if all enemy structures are dead nearby
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
	if (Broodwar->isVisible(getGoal()) &&
		!mpsExplorationManager->hasSpottedBuildingWithinRange(getGoal(), config::squad::attack::STRUCTURES_DESTROYED_GOAL_DISTANCE))
	{
		return true;
	} else {
		return false;
	}
}

bool AttackSquad::isInPosition() const {
	return isCloseTo(getGoal(), config::squad::attack::WAITING_POSITION_DISTANCE_FROM_GOAL);
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

bool AttackSquad::isFollowingAlliedSquad() const {
	return NULL != mpAlliedSquadFollow;
}

AlliedSquadCstPtr AttackSquad::getAlliedSquad() const {
	return mpAlliedSquadFollow;
}

std::string AttackSquad::getName() const {
	return ATTACK_SQUAD_NAME;
}

shared_ptr<AttackSquad> AttackSquad::getThis() const {
	return static_pointer_cast<AttackSquad>(Squad::getThis());
}

#pragma warning(push)
#pragma warning(disable:4100)
void AttackSquad::onWaitGoalAdded(const std::tr1::shared_ptr<WaitGoal>& newWaitGoal) {
	mWaitInPosition = true;
}
#pragma warning(pop)

bool AttackSquad::needsAlliedRegrouping() const {
	if (NULL != mpAlliedSquadFollow || mpAlliedSquadFollow->getCenter() == TilePositions::Invalid) {
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