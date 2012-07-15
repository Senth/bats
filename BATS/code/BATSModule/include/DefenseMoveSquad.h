#pragma once

#include "Squad.h"

// Namespace for the project
namespace bats {

/**
 * A defensive squad that can move around. If it doesn't have a target it will
 * automatically go to position it shall wait in. Once it gets a target position to
 * defend the squad will enter its defending state, only when the enemy retreats from
 * the defending area or when all enemies are defeated it will re-enter its waiting state
 * and move back to the waiting position.
 * This squad will never succeed or fail.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class DefenseMoveSquad : public Squad {
public:
	/**
	 * Constructs the squad with the specified initial units.
	 * @param units initial units to add
	 * @param waitPosition the position for the squad to wait in, defaults to TilePositions::Invalid
	 */
	DefenseMoveSquad(
		const std::vector<UnitAgent*>& units,
		const BWAPI::TilePosition& waitPosition = BWAPI::TilePositions::Invalid
	);

	/**
	 * Destructor
	 */
	virtual ~DefenseMoveSquad();

	/**
	 * Sets a new wait position for the squad.
	 * @param waitPosition new wait position.
	 */
	void setWaitPosition(const BWAPI::TilePosition& waitPosition);

	/**
	 * Tells the squad to go and defend the specified position.
	 * @param defendPosition the position to stay and fend off all enemy units.
	 */
	void defendPosition(const BWAPI::TilePosition& defendPosition);

	/**
	 * Returns true if the squad defends a position
	 * @return true if the squad defends a position
	 */
	bool isDefending() const;

	/**
	 * Returns a DefenseMoveSquad shared_ptr instead of the original Squad shared_ptr.
	 * @note This function is not overridden, but more a helper function to return the
	 * right pointer for an attackSquad.
	 * @return shared_ptr to this AttackSquad
	 */
	DefenseMoveSquadPtr getThis() const;

	virtual std::string getName() const;

protected:
	virtual void updateDerived();

private:
	/**
	 * Checks if the squad is around the defend perimeter.
	 * @return true if the squad is at the defended position
	 */
	bool isWithinDefendPerimeter() const;

	/**
	 * Checks if there are any offensive enemies withing the offensive perimeter.
	 * @return true if there are enemies withing the enemy perimeter
	 * @see findEnemyPositionWithinDefendPerimeter() to find an enemy within
	 * the defend perimeter, i.e. not the enemy perimeter.
	 */
	bool isEnemyWithinOffensivePerimeter() const;

	/**
	 * Checks if there are any enemies alive close to the defended position.
	 * Returns the position to the first enemy found close to the position.
	 * @return position of the first enemy found close to the defended position,
	 * TilePositions::Invalid if no enemies are found.
	 */
	BWAPI::TilePosition findEnemyPositionWithinDefendPerimeter() const;

	BWAPI::TilePosition mDefendPosition;
};
}