#ifndef __SCOUTAGENT_H__
#define __SCOUTAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The ScoutAgent handles Protoss Scout flying units.
 *
 * Implemented special abilities:
 * - To come.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ScoutAgent : public UnitAgent {

private:

public:
	ScoutAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
