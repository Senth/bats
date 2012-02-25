#ifndef __NEXUSAGENT_H__
#define __NEXUSAGENT_H__

#include <BWAPI.h>
#include "StructureAgent.h"

/** The NexusAgent handles Protoss Nexus buildings.
 *
 * Implemented abilities:
 * - Trains and keeps the number of Probes (workers) up.
 * 
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class NexusAgent : public StructureAgent {

private:
	bool hasSentWorkers;

public:
	NexusAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
