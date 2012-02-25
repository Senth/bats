#ifndef __ULTRALISKAGENT_H__
#define __ULTRALISKAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The ZerglingAgent handles Zerg Ultralisk units.
 *
 * Implemented special abilities:
 * - 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class UltraliskAgent : public UnitAgent {

private:

public:
	UltraliskAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
