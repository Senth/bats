#ifndef __UPGRADESPLANNER_H__
#define __UPGRADESPLANNER_H__

#include <BWAPI.h>
#include "BaseAgent.h"

/** UpgradesPlanner contains which updates/techs to be research and in
 * which order.
 *
 * The UpgradesPlanner is implemented as a singleton class. Each class that needs to
 * access UpgradesPlanner can request an instance, and all classes shares the same UpgradesPlanner instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class UpgradesPlanner {

private:
	static UpgradesPlanner* instance;

	UpgradesPlanner();
	
	std::vector<BWAPI::UpgradeType> upgradesP1;
	std::vector<BWAPI::UpgradeType> upgradesP2;
	std::vector<BWAPI::UpgradeType> upgradesP3;
	std::vector<BWAPI::TechType> techsP1;
	std::vector<BWAPI::TechType> techsP2;
	std::vector<BWAPI::TechType> techsP3;

	bool canUpgrade(BWAPI::UpgradeType type, BWAPI::Unit* unit);
	bool canResearch(BWAPI::TechType type, BWAPI::Unit* unit);

public:
	/** Destructor. */
	~UpgradesPlanner();

	/** Returns the instance to the UpgradesPlanner that is currently used. */
	static UpgradesPlanner* getInstance();

	/** Checks if there is an upgrade the specified agent need to upgrade/research. */
	bool checkUpgrade(BaseAgent* agent);
};

#endif
