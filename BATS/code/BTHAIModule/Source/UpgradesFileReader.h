#ifndef __UPGRADESFILEREADER_H__
#define __UPGRADESFILEREADER_H__

#include <BWAPI.h>
#include "FileReaderUtils.h"

/** This class reads the Upgrade/Techs scriptfile.
 * Upgrades/techs are in three priorities, 1 2 and 3. 1 is highest.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class UpgradesFileReader : public FileReaderUtils {

private:
	void addUpgrade(std::string line);
	
	std::vector<BWAPI::UpgradeType> upgradesP1;
	std::vector<BWAPI::UpgradeType> upgradesP2;
	std::vector<BWAPI::UpgradeType> upgradesP3;
	std::vector<BWAPI::TechType> techsP1;
	std::vector<BWAPI::TechType> techsP2;
	std::vector<BWAPI::TechType> techsP3;

public:
	/** Constructor. */
	UpgradesFileReader();

	/** Reads the upgrades from file.*/
	void readUpgrades();

	/** Returns upgrades prio 1. */
	std::vector<BWAPI::UpgradeType> getUpgradesP1();
	/** Returns upgrades prio 2. */
	std::vector<BWAPI::UpgradeType> getUpgradesP2();
	/** Returns upgrades prio 3. */
	std::vector<BWAPI::UpgradeType> getUpgradesP3();
	/** Returns techs prio 1. */
	std::vector<BWAPI::TechType> getTechsP1();
	/** Returns techs prio 2. */
	std::vector<BWAPI::TechType> getTechsP2();
	/** Returns techs prio 3. */
	std::vector<BWAPI::TechType> getTechsP3();
};

#endif
