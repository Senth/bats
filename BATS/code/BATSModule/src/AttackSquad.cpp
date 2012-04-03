#include "AttackSquad.h"
#include "AttackCoordinator.h"
#include "Config.h"
#include "ExplorationManager.h"

using namespace bats;
using BWAPI::TilePosition;
using BWAPI::Broodwar;
using namespace std::tr1;

AttackCoordinator* AttackSquad::mpsAttackCoordinator = NULL;
ExplorationManager* AttackSquad::mpsExplorationManager = NULL;

AttackSquad::AttackSquad(const std::vector<UnitAgent*> units, bool distracting) : Squad(units) {
	mDistraction = distracting;
	mWaitInPosition = false;

	if (mpsAttackCoordinator == NULL) {
		mpsAttackCoordinator = AttackCoordinator::getInstance();
		mpsExplorationManager = ExplorationManager::getInstance();
	}
}

AttackSquad::~AttackSquad() {
	// Does nothing
}

void AttackSquad::computeSquadSpecificActions() {
	// Create a wait position when we're in position, if we shall wait that is.
	if (mWaitInPosition && isInPosition()) {
		if (hasTemporaryGoalPosition()) {
			// All wait goals done, continue to goal
			if (!hasWaitGoals()) {
				mWaitInPosition = false;
				updateUnitGoals();
			}
		} else {
			setTemporaryGoalPosition(getCenter());
		} 
	}
}

bool AttackSquad::createGoal() {
	shared_ptr<AttackSquad> thisAttackSquad = getThis();
	mpsAttackCoordinator->requestAttack(thisAttackSquad);
	return true;
}

Squad::GoalStates AttackSquad::checkGoalState() const {
	Squad::GoalStates goalState = Squad::GoalState_NotCompleted;

	// Check if all enemy structures are dead nearby
	if (Broodwar->isVisible(getGoal()) &&
		mpsExplorationManager->hasSpottedBuilding() &&
		!mpsExplorationManager->hasSpottedBuildingWithinRange(getGoal(), config::squad::attack::STRUCTURES_DESTROYED_GOAL_DISTANCE)) {
		goalState = Squad::GoalState_Succeeded;
	}

	/// @todo check if enemy force is too strong.

	/// @todo check for enemy units to kill them?

	return goalState;
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

shared_ptr<AttackSquad> AttackSquad::getThis() const {
	return static_pointer_cast<AttackSquad>(Squad::getThis());
}

#pragma warning(push)
#pragma warning(disable:4100)
void AttackSquad::onWaitGoalAdded(const std::tr1::shared_ptr<WaitGoal>& newWaitGoal) {
	mWaitInPosition = true;
}
#pragma warning(pop)