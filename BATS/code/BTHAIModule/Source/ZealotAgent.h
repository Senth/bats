#ifndef __ZEALOTAGENT_H__
#define __ZEALOTAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The ZealotAgent handles Protoss Zealot units.
 *
 * Implemented special abilities:
 * - 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ZealotAgent : public UnitAgent {

private:

public:
	ZealotAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
