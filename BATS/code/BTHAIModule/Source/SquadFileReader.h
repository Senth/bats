#ifndef __SQUADFILEREADER_H__
#define __SQUADFILEREADER_H__

#include <BWAPI.h>
#include "Squad.h"
#include "FileReaderUtils.h"

/** This class reads the squad setup script files.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class SquadFileReader : public FileReaderUtils {

private:
	void addSquad(std::string line, std::vector<BWAPI::UnitType> &squads);
	void addUnit(std::string line);
	void createSquad();

	std::string type;
	std::string name;
	std::string offType;
	std::string requirement;
	int priority;
	int activePriority;
	BWAPI::UnitType morphsTo;

	Squad* cSquad;
	int id;

	std::vector<Squad*> squads;

public:
	/** Constructor. */
	SquadFileReader();

	/** Reads the squad setup script file. */
	std::vector<Squad*> readSquadList();
};

#endif
