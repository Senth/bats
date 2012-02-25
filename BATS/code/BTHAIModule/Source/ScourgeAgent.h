#ifndef __SCOURGEAGENT_H__
#define __SCOURGEAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The ScourgeAgent handles Zerg Scourge units.
 *
 * Implemented special abilities:
 * - 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ScourgeAgent : public UnitAgent {

private:

public:
	ScourgeAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
