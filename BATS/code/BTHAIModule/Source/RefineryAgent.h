#pragma once

#include "StructureAgent.h"
#include <vector>

class WorkerAgent;

/** The RefineryAgent handles Refinery buildings for all races.
 *
 * Implemented abilities:
 * - Makes sure each Refinery has 3 workers assigned to gather gas.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 */
class RefineryAgent : public StructureAgent {

private:
	std::vector<WorkerAgent*> assignedWorkers;

public:
	RefineryAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};
