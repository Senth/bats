#ifndef __KITESQUAD_H__
#define __KITESQUAD_H__

#include "BaseAgent.h"
#include "Squad.h"

/** This is the same as RushSquad, except that agents kite enemy units once
 * they have been attacked.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class KiteSquad : public Squad {

private:
	BWAPI::TilePosition getNextStartLocation();
	bool isVisible(BWAPI::TilePosition pos);
	std::vector<BWAPI::TilePosition> hasVisited;
	
public:
	/** Constructor. See Squad.h for more details. */
	KiteSquad(int mId, std::string mName, int mPriority);

	/** Returns true if this Squad is active, or false if not.
	 * A Squad is active when it first has been filled with agents.
	 * A Squad with destroyed units are still considered Active. */
	bool isActive();

	/** Called each update to issue orders. */
	void computeActions();

	/** Orders this squad to defend a position. */
	void defend(BWAPI::TilePosition mGoal);

	/** Orders this squad to launch an attack at a position. */
	void attack(BWAPI::TilePosition mGoal);

	/** Orders this squad to assist units at a position. */
	void assist(BWAPI::TilePosition mGoal);

	/** Clears the goal for this Squad, i.e. sets the goal
	 * to BWAPI::TilePosition(-1,-1). */
	void clearGoal();

	/** Returns the current goal of this Squad. */
	BWAPI::TilePosition getGoal();

	/** Returns true if this squad has an assigned goal. */
	bool hasGoal();

	/** Prints some info about the squad. */
	void printInfo();
};

#endif
