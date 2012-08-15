#ifndef __PFMANAGER_H__
#define __PFMANAGER_H__

#include <BWAPI.h>

class BaseAgent;
class UnitAgent;

/** In the bot unit navigation uses two techniques; if no enemy units are close units navigate using the built in pathfinder in
 * Starcraft. If enemy units are close, own units uses potential fields to engage and surround the enemy.
 * The PFManager class is the main class for the potential fields navigation system, and it shall be used compute and execute
 * movement orders using potential fields.
 *
 * The PFManager is implemented as a singleton class. Each class that needs to access PFManager can request an instance,
 * and all classes shares the same PFManager instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class PFManager {

private:
	PFManager();
	static PFManager* instance;
	static bool instanceFlag;

	float getAttackingUnitP(const BaseAgent* agent, int cX, int cY, bool defensive);
	
	int checkRange;
	int stepSize;
	int mapW;
	int mapH;

	BWAPI::Color getColor(float p);

public:
	/** Destructor */
	~PFManager();

	/** Returns the instance to the class. */
	static PFManager* getInstance();

	/** Is used to compute and execute movement commands for attacking units using the potential field
	 * navigation system. If forceMove is set to true, units always move even if they can attack. */
	void computeAttackingUnitActions(BaseAgent* agent, BWAPI::TilePosition goal, bool defensive, bool forceMove = false);

	/** Displays a debug view of the potential fields for an agent. */
	void displayPF(const UnitAgent* agent);

	/** Moves a unit to the specified goal using the pathfinder, and stops at a distance where the
	* potential field navigation system should be used instead. */
	bool moveToGoal(BaseAgent* agent, const BWAPI::TilePosition& goal, bool defensive = false, bool forceMove = false);
	
};

#endif
