#pragma once

#include <BWAPI.h>
#include "BTHAIModule/Source/FileReaderUtils.h"
#include "BuildPlanner.h"
#include "UnitCreator.h"

using namespace std;
// Namespace for the project
namespace bats {


/**
 * Manages build orders in three phases: early, mid, and late game. For both buildings and units
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 */
class BuildOrderFileReader : public FileReaderUtils {

private:
	/**
	* Used for storing the percentage units and must have units from the build order file
	*/
	vector<bats::CoreUnit> mCoreUnitsList;
	/**
	* Adds Building to the build order list.
	*/	
	void addBuildingType(std::string line, std::vector<BWAPI::UnitType> &buildOrder);

	/**
	* Adds unit to the Core Units list.
	*/	
	void addUnitType(std::string line, std::vector<bats::CoreUnit> &coreUnits, bool mustHave);

public:
	BuildOrderFileReader();

	/** Reads the buildorder from script file. 
	* @param phase same as folder name
	* @param fileName is name of the file
	* @param buildOrder in order to queue the next phase if theres is left overs from previous phase
	*/
	vector<BWAPI::UnitType> readBuildOrder(string phase, string fileName, vector<BWAPI::UnitType> &buildOrder);

	/**
	Used only after calling readBuildOrder member method
	Getter method to get the unit list read from the build order configuration file
	*/
	vector<bats::CoreUnit> getUnitList();
	
	/** Reads the transition file.
	* Creates TransitionGraph with early, mid and late phase order from the file
	*/
	TransitionGraph readTransitionFile(string fileName);

	/** Used for getting the Units, read from the build order file (the phase is maintained by BuildPlanner)
	*/
	vector<BWAPI::UnitType> readUnitsWithPercentage;
};
}