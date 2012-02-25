#ifndef __TARGETINGAGENT_H__
#define __TARGETINGAGENT_H__

#include <BWAPI.h>
#include "BaseAgent.h"

/** This agent is used to find the best target to attack for a unit. 
 * It is possible to add several rules, for example always target workers first.
 *
 * Currently the most expensive target (highest destroyscore) in range is targeted.
 * Units and buildings that cannot attack back get reduced score.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TargetingAgent {

private:
	static double getTargetModifier(BWAPI::UnitType attacker, BWAPI::UnitType target);
	static bool isCloakingUnit(BWAPI::UnitType type);

public:
	/** Returns the best target within seekrange for a unit agent, or NULL if no target
	 * was found. */
	static BWAPI::Unit* findTarget(BaseAgent* agent);
};

#endif
