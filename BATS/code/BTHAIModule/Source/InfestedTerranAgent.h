#ifndef __INFESTEDTERRANAGENT_H__
#define __INFESTEDTERRANAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The InfestedTerranAgent handles Zerg Infested Terran units.
 *
 * Implemented special abilities:
 * - 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class InfestedTerranAgent : public UnitAgent {

private:

public:
	InfestedTerranAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
