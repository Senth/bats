#include "AttackSquad.h"

using namespace bats;

AttackSquad::AttackSquad(const std::vector<UnitAgent*> units) : Squad(units) {
	// Does nothing
}

AttackSquad::~AttackSquad() {
	// Does nothing
}

void AttackSquad::computeSquadSpecificActions() {

}

void AttackSquad::createGoal() {
	// TODO
}

Squad::GoalStates AttackSquad::getGoalState() const {
	/// @todo remove this function, add the goal checking in compute squad specific actions instead.
	return GoalState_NotCompleted;
}