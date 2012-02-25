#ifndef __CORSAIRAGENT_H__
#define __CORSAIRAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The CorsairAgent handles Protoss Corsair flying units.
 *
 * Implemented special abilities:
 * - If researched, uses Disruption Web to take out enemy air defense.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class CorsairAgent : public UnitAgent {

private:
	int lastUseFrame;

public:
	CorsairAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
