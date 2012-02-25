#ifndef __FILEREADERUTILS_H__
#define __FILEREADERUTILS_H__

#include <BWAPI.h>

struct Tokens {
	std::string key;
	std::string value;
};

/** This class contains some common methods used by classes handling the
 * reading of buildorder/techs/upgrades/squad setup files.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class FileReaderUtils {

private:
	
public:
	FileReaderUtils();

	/** Returns the filename to use in sub folder squads, buildorder or upgrades.
	 * The methods checks if for example PvZ is defined, and if not PvX is used. */
	std::string getFilename(std::string subpath);

	/** Returns the path to the folder where the scripfiles are placed. */
	std::string getScriptPath();

	/** Checks if a file in the specified subpath exists, for example PvZ.txt in
	 * subfolder buildorder. */
	bool fileExists(std::string subpath, std::string filename);

	/** Returns a unit type from a textline, or Unknown if no type was found. */
	BWAPI::UnitType getUnitType(std::string line);

	/** Returns an upgrade type from a textline, or Unknown if no type was found. */
	BWAPI::UpgradeType getUpgradeType(std::string line);

	/** Returns a tech type from a textline, or Unknown if no type was found. */
	BWAPI::TechType getTechType(std::string line);

	/** Replaces all underscores (_) with whitespaces in a std::string. */
	void replace(std::string &line);

	/** Splits a line into tokens. Delimiter is the characted to split at, for example = or :. */
	Tokens split(std::string line, std::string delimiter);

	/** Converts a std::string to an int. */
	int toInt(std::string &str);

};

#endif
