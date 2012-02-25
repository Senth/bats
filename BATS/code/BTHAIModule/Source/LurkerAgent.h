#ifndef __LURKERAGENT_H__
#define __LURKERAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The LurkerAgent handles Zerg Lurker units.
 *
 * Implemented special abilities:
 * - Uses burrow when enemies are nearby.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class LurkerAgent : public UnitAgent {

private:

public:
	LurkerAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
