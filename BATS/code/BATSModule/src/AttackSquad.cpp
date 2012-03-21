#include "AttackSquad.h"

using namespace bats;
using BWAPI::TilePosition;
using BWAPI::Broodwar;

AttackSquad::AttackSquad(const std::vector<UnitAgent*> units) : Squad(units) {
	// Does nothing
}

AttackSquad::~AttackSquad() {
	// Does nothing
}

void AttackSquad::computeSquadSpecificActions() {

}

bool AttackSquad::createGoal() {
	// Go to center of map for now
	TilePosition position;
	position.x() = Broodwar->mapWidth() / 2;
	position.y() = Broodwar->mapHeight() / 2;
	setGoalPosition(position);
	return true;
}