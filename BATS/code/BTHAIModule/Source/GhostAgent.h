#ifndef __GHOSTAGENT_H__
#define __GHOSTAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The GhostAgent handles Terran Ghost units.
 *
 * Implemented special abilities:
 * - Uses Personel Cloaking when needed.
 * - Uses Lockdown on expensive enemy tanks.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class GhostAgent : public UnitAgent {

private:
	BWAPI::Unit* findLockdownTarget();

public:
	GhostAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
