#pragma once

#include "Squad.h"

// Namespace for the project
namespace bats {

/**
 * A defensive patrol squad that can move around. If it's not defending any area it will
 * automatically patrol the selected areas. Currently it patrols the areas in no specific
 * order, but not random. \n
 * \n
 * Once it gets a target position to
 * defend the squad will enter its defending state, only when the enemy retreats from
 * the defending area or when all enemies are defeated it will re-enter its patrol state
 * and continue patrolling. \n
 * \n
 * This squad will neither succeed nor fail.
 * @todo Create the most efficient path for patrolling through the patrolling positions.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class PatrolSquad : public Squad {
public:
	/**
	 * Constructs the squad with the specified initial units.
	 * @param units initial units to add
	 * @param waitPosition the position for the squad to wait in, defaults to TilePositions::Invalid
	 */
	PatrolSquad(
		const std::vector<UnitAgent*>& units,
		const BWAPI::TilePosition& waitPosition = BWAPI::TilePositions::Invalid
	);

	/**
	 * Destructor
	 */
	virtual ~PatrolSquad();

	/**
	 * Set the patrolling positions.
	 * @param patrolPositions new position to patrol between
	 * @note It will not reset the positions and make the squad begin to move to the first
	 * position again.
	 */
	void setPatrolPositions(const std::set<BWAPI::TilePosition>& patrolPositions);

	/**
	 * Tells the squad to go and defend the specified position.
	 * @param defendPosition the position to stay and fend off all enemy units.
	 * @param defendEnemyOffensivePerimeter set this to true if you want the squad to defend the 
	 * area within enemy_offensive_perimeter. Useful when defending inside our base,
	 * then we don't want to hold back where we're going. Defaults to false.
	 */
	void defendPosition(const BWAPI::TilePosition& defendPosition, bool defendEnemyOffensivePerimeter = false);

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
	PatrolSquadCstPtr getThis() const;

	/**
	 * \copydoc getThis() const
	 */
	PatrolSquadPtr getThis();

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
	 * Goes to the next patrol position. If none exist it will do nothing.
	 */
	void goToNextPatrolPosition();

	/**
	 * Handles patrolling, going to the next position when close to the current one.
	 */
	void handlePatrol();

	/**
	 * Handles defending an areas, checks when the defending has been successfully completed.
	 * Assigns where the squad shall attack if enemies move close enough.
	 */
	void handleDefend();

	BWAPI::TilePosition mDefendPosition;
	bool mDefendEnemyOffensivePerimeter;
	std::set<BWAPI::TilePosition>::iterator mPatrolPositionCurrentIt;
	std::set<BWAPI::TilePosition> mPatrolPositions;
};
}