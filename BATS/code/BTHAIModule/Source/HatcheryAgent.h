#ifndef __HATCHERYAGENT_H__
#define __HATCHERYAGENT_H__

#include <BWAPI.h>
#include "StructureAgent.h"

/** The HatcheryAgent handles Zerg Hatchery/Lair/Hive buildings.
 *
 * Implemented abilities:
 * - Trains and keeps the number of workers up.
 * - Can morph to Lair and Hive.
 * 
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class HatcheryAgent : public StructureAgent {

private:
	bool hasSentWorkers;
	bool checkBuildUnit(BWAPI::UnitType type);

public:
	HatcheryAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
