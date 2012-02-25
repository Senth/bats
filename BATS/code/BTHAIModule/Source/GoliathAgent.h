#ifndef __GOLIATHAGENT_H__
#define __GOLIATHAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The GoliathAgent handles Terran Goliath units.
 *
 * Implemented special abilities:
 * - Targets air unit first if there are any nearby.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class GoliathAgent : public UnitAgent {

private:

public:
	GoliathAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
