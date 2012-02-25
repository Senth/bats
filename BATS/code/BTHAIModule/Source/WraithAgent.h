#ifndef __WRAITHAGENT_H__
#define __WRAITHAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The WraithAgent handles Terran Wraith flying units.
 *
 * Implemented special abilities:
 * - If there are enemy units within firerange, the Wraith uses Cloaking 
 * Field (if researched).
 * - Wraiths can be used as explorers.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class WraithAgent : public UnitAgent {

private:
	bool checkUseCloak();
public:
	WraithAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
