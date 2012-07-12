#include "DefenseMoveSquad.h"

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
	/// @todo
	return false;
}

bool DefenseMoveSquad::isEnemyWithinOffensivePerimeter() const {
	/// @todo
	return false;
}

BWAPI::TilePosition DefenseMoveSquad::findEnemyPositionWithinDefendPerimeter() const {
	/// @todo
	return TilePositions::Invalid;
}

void DefenseMoveSquad::printGraphicDebugInfo() {
	/// @todo print circle for the defense perimeter
}