#include "DefenseMoveSquad.h"
#include "Config.h"

using namespace bats;
using namespace BWAPI;

const std::string DEFENSE_MOVE_SQUAD_NAME = "DefenseMoveSquad";

DefenseMoveSquad::DefenseMoveSquad(
	const std::vector<UnitAgent*>& units,
	const BWAPI::TilePosition& waitPosition)
	:
	Squad(units)
{
	setCanRegroup(false);
	setGoalPosition(waitPosition);

	mDefendPosition = TilePositions::Invalid;
}

DefenseMoveSquad::~DefenseMoveSquad() {
	/// @todo
}

void DefenseMoveSquad::setWaitPosition(const BWAPI::TilePosition& waitPosition) {
	setGoalPosition(waitPosition);
}

void DefenseMoveSquad::defendPosition(const BWAPI::TilePosition& defendPosition) {
	mDefendPosition = defendPosition;
}

void DefenseMoveSquad::updateDerived() {
	// Don't do anything if we aren't defending
	if (!isDefending()) {
		return;
	}


	// Squad is outside defend position perimeter -> Go to defend position
	if (!isWithinDefendPerimeter()) {
		setTemporaryGoalPosition(mDefendPosition);
	}	
	// Else - Squad is within defend perimeter
	else {
		// Find enemy positions to go to
		const BWAPI::TilePosition& attackPosition = findEnemyPositionWithinDefendPerimeter();

		// No enemy position found with defend perimeter
		if (!attackPosition.isValid()) {
			// Enemies still within offensive perimeter -> regroup at defend position
			if (isEnemyWithinOffensivePerimeter()) {
				setTemporaryGoalPosition(mDefendPosition);
			}
			// Else - Enemies retreated or dead -> Go back to wait position
			else {
 				mDefendPosition = TilePositions::Invalid;
				setTemporaryGoalPosition(TilePositions::Invalid);
			}
		} else {
			setTemporaryGoalPosition(attackPosition);
		}
	}
}

DefenseMoveSquadPtr DefenseMoveSquad::getThis() const {
	return std::tr1::static_pointer_cast<DefenseMoveSquad>(Squad::getThis());
}

std::string DefenseMoveSquad::getName() const {
	return DEFENSE_MOVE_SQUAD_NAME;
}

bool DefenseMoveSquad::isDefending() const {
	return mDefendPosition != TilePositions::Invalid;
}

bool DefenseMoveSquad::isWithinDefendPerimeter() const {
	return isCloseTo(mDefendPosition, config::squad::defend::PERIMETER);
}

bool DefenseMoveSquad::isEnemyWithinOffensivePerimeter() const {
	const std::set<Unit*>& units = Broodwar->getUnitsInRadius(Position(mDefendPosition), config::squad::defend::ENEMY_OFFENSIVE_PERIMETER * TILE_SIZE);
	std::set<Unit*>::const_iterator unitIt;
	for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
		if ((*unitIt)->getPlayer()->isEnemy(Broodwar->self())) {
			return true;
		}
	}

	return false;
}

BWAPI::TilePosition DefenseMoveSquad::findEnemyPositionWithinDefendPerimeter() const {
	const std::set<Unit*>& units = Broodwar->getUnitsInRadius(Position(mDefendPosition), config::squad::defend::PERIMETER * TILE_SIZE);
	std::set<Unit*>::const_iterator unitIt;
	for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
		if ((*unitIt)->getPlayer()->isEnemy(Broodwar->self()) && (*unitIt)->getTilePosition().isValid()) {
			return (*unitIt)->getTilePosition();
		}
	}

	return TilePositions::Invalid;
}