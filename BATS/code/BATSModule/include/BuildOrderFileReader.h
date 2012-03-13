#pragma once

#include <BWAPI.h>
#include "BTHAIModule/Source/FileReaderUtils.h"
#include "BuildPlanner.h"

using namespace std;
// Namespace for the project
namespace bats {


/**
 * Manages build orders in three phases: early, mid, and late game.
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 */
class BuildOrderFileReader : public FileReaderUtils {

private:
	void addUnitType(std::string line, std::vector<BWAPI::UnitType> &buildOrder);

public:
	BuildOrderFileReader();

	/** Reads the buildorder from script file. 
	* phase : same as folder name
	* buildOrder : in order to queue the next phase if theres is left overs from previous phase
	*/
	vector<BWAPI::UnitType> readBuildOrder(string phase, string fileName, vector<BWAPI::UnitType> &buildOrder);
	TransitionGraph readTransitionFile(string fileName);
		
};
}