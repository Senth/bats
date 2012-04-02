#include "AttackSquad.h"
#include "AttackCoordinator.h"

using namespace bats;
using BWAPI::TilePosition;
using BWAPI::Broodwar;
using namespace std::tr1;

AttackCoordinator* AttackSquad::mpsAttackCoordinator = NULL;

AttackSquad::AttackSquad(const std::vector<UnitAgent*> units, bool distracting) : Squad(units) {
	mDistraction = distracting;

	if (mpsAttackCoordinator == NULL) {
		mpsAttackCoordinator = AttackCoordinator::getInstance();
	}
}

AttackSquad::~AttackSquad() {
	// Does nothing
}

void AttackSquad::computeSquadSpecificActions() {
	/// @todo check if we have a wait goal, then create a wait position.

	/// @todo check if we're done with the goal
}

bool AttackSquad::createGoal() {
	shared_ptr<AttackSquad> thisAttackSquad = getThis();
	mpsAttackCoordinator->requestAttack(thisAttackSquad);
	return true;
}

bool AttackSquad::isInPosition() const {
	/// @todo check if squad is in position
	return true;
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