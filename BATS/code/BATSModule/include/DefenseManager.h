#pragma once

#include <vector>
#include "TypeDefs.h"

// Forward declaration
namespace BWAPI {
	class Unit;
}

// Namespace for the project
namespace bats {

// Forward declaration
class UnitManager;
class SquadManager;

/**
 * Manages the defense for the team. Creates DefensiveHoldSquad and DefensivePatrolSquad
 * and checks which areas shall be defended, both around our base and around the player's
 * base.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class DefenseManager {
public:
	/**
	 * Destructor
	 */
	virtual ~DefenseManager();

	/**
	 * Returns the instance of DefensiveManager.
	 * @return instance of DefensiveManager.
	 */
	static DefenseManager* getInstance();

	/**
	 * Checks for places that needs defending and for free units to add to defending
	 * squads.
	 */
	void update();

	/**
	 * Returns all units that are free to use for another squad. This generally means all
	 * units in the DefensivePatrolSquad and DefensiveHoldSquads if we're not under attack.
	 * If we're under attack it will return no units.
	 * @return returns all free units that aren't occupied with defending. I.e. all or none.
	 */
	std::vector<BWAPI::Unit*> getAllFreeUnits();

	/**
	 * Checks whether either we or the player is under attack.
	 * @return true if we or the player is under attack.
	 */
	bool isUnderAttack() const;

	/**
	 * Prints graphical debug information.
	 * \li Defense perimeter
	 * \li Enemy offensive perimeter
	 */
	void printGraphicDebugInfo() const;

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	DefenseManager();

	/**
	 * Searches for and finds all choke points that needs defending.
	 */
	void updateDefendPositions();

	UnitManager* mpUnitManager;
	SquadManager* mpSquadManager;

	static DefenseManager* mpsInstance;
};
}