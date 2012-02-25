#ifndef __SIEGETANKAGENT_H__
#define __SIEGETANKAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The SiegeTankAgent handles Terran Siege Tank units.
 *
 * Implemented special abilities:
 * - If there are enemy units within firerange, the tank enters Siege mode (if researched).
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class SiegeTankAgent : public UnitAgent {
	
public:
	SiegeTankAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
