#ifndef __RUSHSQUAD_H__
#define __RUSHSQUAD_H__

#include "BaseAgent.h"
#include "Squad.h"

/** This squad rushes to each start location until the enemy has been located. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class RushSquad : public Squad {

private:
	BWAPI::Unit* findWorkerTarget();
	
public:
	/** Constructor. See Squad.h for more details. */
	RushSquad(int mId, std::string mName, int mPriority);

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
