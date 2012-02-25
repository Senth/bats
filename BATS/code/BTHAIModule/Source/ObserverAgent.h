#ifndef __OBSERVERAGENT_H__
#define __OBSERVERAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The ObserverAgent handles Protoss Observer cloaked flying units.
 *
 * Implemented special abilities:
 * - Observers are used as detectors in the attack force.
 * - Observers can be used as explorers.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ObserverAgent : public UnitAgent {

private:
	
public:
	ObserverAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
