#ifndef __OVERLORDAGENT_H__
#define __ZEALOTAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The OverlordAgent handles Zerg Overlord units.
 *
 * Implemented special abilities:
 * - 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class OverlordAgent : public UnitAgent {

private:
	int lastUpdateFrame;
	void updateGoal();

public:
	OverlordAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
