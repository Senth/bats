#ifndef __BUILDORDERFILEREADER_H__
#define __BUILDORDERFILEREADER_H__

#include <BWAPI.h>
#include "FileReaderUtils.h"

/** This file reads the buildorder script files.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BuildOrderFileReader : public FileReaderUtils {

private:
	void addUnitType(std::string line, std::vector<BWAPI::UnitType> &buildOrder);

public:
	BuildOrderFileReader();

	/** Reads the buildorder from script file. */
	std::vector<BWAPI::UnitType> readBuildOrder();
};

#endif
