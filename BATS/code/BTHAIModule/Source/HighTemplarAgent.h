#ifndef __HIGHTEMPLARAGENT_H__
#define __HIGHTEMPLARAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The HighTemplarAgent handles Protoss High Templar units.
 *
 * Implemented special abilities:
 * - Can use all spells.
 * - Can merge to Archons (not well tested).
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class HighTemplarAgent : public UnitAgent {

private:
	BWAPI::Unit* findPsiStormTarget();
	BaseAgent* findHallucinationTarget();
	BaseAgent* findArchonTarget();
	bool hasCastTransform;

public:
	HighTemplarAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
