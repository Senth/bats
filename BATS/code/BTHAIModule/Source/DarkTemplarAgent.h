#ifndef __DARKTEMPLARAGENT_H__
#define __DARKTEMPLARAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The DarkTemplarAgent handles Protoss Dark Templar units.
 *
 * Implemented special abilities:
 * - Avoids detectors.
 * - Are most effective in HarassSquads.
 *
 * TODO:
 * - Merge to Dark Archon
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class DarkTemplarAgent : public UnitAgent {

private:

public:
	DarkTemplarAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
