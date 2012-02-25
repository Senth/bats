#ifndef __REFINERYAGENT_H__
#define __REFINERYAGENT_H__

#include "StructureAgent.h"

/** The RefineryAgent handles Refinery buildings for all races.
 *
 * Implemented abilities:
 * - Makes sure each Refinery has 3 workers assigned to gather gas.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class RefineryAgent : public StructureAgent {

private:
	std::vector<BaseAgent*> assignedWorkers;

public:
	RefineryAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
