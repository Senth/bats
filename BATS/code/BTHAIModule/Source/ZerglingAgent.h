#ifndef __ZERGLINGAGENT_H__
#define __ZERGLINGAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The ZerglingAgent handles Zerg Zergling units.
 *
 * Implemented special abilities:
 * - 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ZerglingAgent : public UnitAgent {

private:

public:
	ZerglingAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
