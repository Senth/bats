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
	// Go to center of map for now
	//TilePosition position;
	//position.x() = Broodwar->mapWidth() / 2;
	//position.y() = Broodwar->mapHeight() / 2;
	//setGoalPosition(position);

	shared_ptr<AttackSquad> thisAttackSquad = dynamic_pointer_cast<AttackSquad>(getThis());
	mpsAttackCoordinator->requestAttack(thisAttackSquad);
	return true;
}

bool AttackSquad::isInPosition() const {
	/// @todo check if squad is in position
	return true;
}

bool AttackSquad::isAttacking() const {
	/// @todo check if squad is attacking or under attack
	return true;
}

bool AttackSquad::isReadyToAttack() const {
	return isInPosition() || isAttacking();
}

bool AttackSquad::isDistracting() const {
	return mDistraction;
}