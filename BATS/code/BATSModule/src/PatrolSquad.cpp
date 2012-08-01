#include "PatrolSquad.h"
#include "Config.h"
#include "Helper.h"

using namespace bats;
using namespace BWAPI;

const std::string PATROL_SQUAD_NAME = "PatrolSquad";

PatrolSquad::PatrolSquad(
	const std::vector<UnitAgent*>& units,
	const BWAPI::TilePosition& waitPosition)
	:
	Squad(units)
{
	setCanRegroup(false);
	setGoalPosition(waitPosition);

	mDefendPosition = TilePositions::Invalid;
	mPatrolPositionCurrentIt = mPatrolPositions.end();
}

PatrolSquad::~PatrolSquad() {
	// Does nothing
}

void PatrolSquad::setPatrolPositions(const std::set<BWAPI::TilePosition>& patrolPositions) {
	// Remove all positions not found in the new set
	std::set<BWAPI::TilePosition>::iterator oldPositionIt = mPatrolPositions.begin();
	while (oldPositionIt != mPatrolPositions.end()) {
		std::set<BWAPI::TilePosition>::const_iterator foundIt = patrolPositions.find(*oldPositionIt);
		
		// Not found -> remove
		if (foundIt == patrolPositions.end()) {
			// Found it is same current patrol position -> use next
			if (oldPositionIt == mPatrolPositionCurrentIt) {
				goToNextPatrolPosition();

				// If still the same -> Remove
				if (oldPositionIt == mPatrolPositionCurrentIt) {
					mPatrolPositionCurrentIt = mPatrolPositions.end();
				}
			}

			oldPositionIt = mPatrolPositions.erase(oldPositionIt);
		} else {
			++oldPositionIt;
		}
	}
	
	
	// Add new positions
	std::set<BWAPI::TilePosition>::const_iterator newPositionIt;
	for (newPositionIt = patrolPositions.begin(); newPositionIt != patrolPositions.end(); ++newPositionIt) {
		mPatrolPositions.insert(*newPositionIt);
	}


	// We didn't have a current patrol position -> use next
	if (mPatrolPositionCurrentIt == mPatrolPositions.end()) {
		goToNextPatrolPosition();
	}
}

void PatrolSquad::defendPosition(const BWAPI::TilePosition& defendPosition) {
	mDefendPosition = defendPosition;
}

void PatrolSquad::updateDerived() {
	if (isDefending()) {
		handleDefend();
	} else {
		handlePatrol();
	}
}

void PatrolSquad::handlePatrol() {
	// Returns if we don't have any target to patrol to
	if (mPatrolPositionCurrentIt == mPatrolPositions.end()) {
		return;
	}


	// Check if we're close to the patrol position
	if (isAUnitStill() || isCloseTo(*mPatrolPositionCurrentIt)) {
		goToNextPatrolPosition();
	}
}

void PatrolSquad::handleDefend() {
	// Squad is outside defend position perimeter -> Go to defend position
	if (!isWithinDefendPerimeter()) {
		setTemporaryGoalPosition(mDefendPosition);
	}	
	// Else - Squad is within defend perimeter
	else {
		// Find enemy positions to go to
		const BWAPI::TilePosition& attackPosition =
			findEnemyPositionWithinRadius(mDefendPosition, config::squad::defend::PERIMETER);

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

PatrolSquadPtr PatrolSquad::getThis() const {
	return std::tr1::static_pointer_cast<PatrolSquad>(Squad::getThis());
}

std::string PatrolSquad::getName() const {
	return PATROL_SQUAD_NAME;
}

bool PatrolSquad::isDefending() const {
	return mDefendPosition != TilePositions::Invalid;
}

bool PatrolSquad::isWithinDefendPerimeter() const {
	return isCloseTo(mDefendPosition, config::squad::defend::PERIMETER);
}

bool PatrolSquad::isEnemyWithinOffensivePerimeter() const {
	return isEnemyWithinRadius(mDefendPosition, config::squad::defend::ENEMY_OFFENSIVE_PERIMETER);
}

void PatrolSquad::goToNextPatrolPosition() {
	// Use next if available
	if (mPatrolPositionCurrentIt != mPatrolPositions.end()) {
		++mPatrolPositionCurrentIt;
	}

	// No current position OR at end, use first one
	if (mPatrolPositionCurrentIt == mPatrolPositions.end()) {
		mPatrolPositionCurrentIt = mPatrolPositions.begin();
	}

	// Set position
	if (mPatrolPositionCurrentIt != mPatrolPositions.end()) {
		setGoalPosition(*mPatrolPositionCurrentIt);
	}
}