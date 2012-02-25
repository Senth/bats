#ifndef __QUEENAGENT_H__
#define __QUEENAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The QueenAgent handles Zerg Queen units.
 *
 * Implemented special abilities:
 * - Can spawn broodlings
 * - Can Ensnare an area
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class QueenAgent : public UnitAgent {

private:
	bool checkSpawnBroodlings();

public:
	QueenAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
