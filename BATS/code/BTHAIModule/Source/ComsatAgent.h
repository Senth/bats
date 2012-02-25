#ifndef __COMSATAGENT_H__
#define __COMSATAGENT_H__

#include "StructureAgent.h"

/** The ComsatAgent handles Terran Comsat Station buildings.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ComsatAgent : public StructureAgent {

private:
	
public:
	ComsatAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();
};

#endif
