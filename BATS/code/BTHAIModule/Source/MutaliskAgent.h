#ifndef __MUTALISKAGENT_H__
#define __MUTALISKAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The MutaliskAgent handles Zerg Mutalisk units.
 *
 * Implemented special abilities:
 * - Can morph into Guardians.
 * - Can morph into Devourers.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class MutaliskAgent : public UnitAgent {

private:

public:
	MutaliskAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
