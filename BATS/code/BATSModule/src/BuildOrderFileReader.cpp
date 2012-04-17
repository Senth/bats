#include "BuildOrderFileReader.h"
#include "BTHAIModule/Source/ExplorationManager.h"
#include "BuildPlanner.h"
#include "Utilities/Logger.h"
#include "Utilities/String.h"
#include "Config.h"
#include <fstream>
#include <sstream>

using namespace BWAPI;
using namespace std;
using namespace bats;

const string BUILD_ORDER_EXT = ".txt";

BuildOrderFileReader::BuildOrderFileReader(){
	//TODO: read the transition config file
}

TransitionGraph BuildOrderFileReader::readTransitionFile(string fileName){
	
	//Read transition file
	ifstream inFile;
	TransitionGraph graph;
	string filePath = config::build_order::DIR + fileName;
	Tokens token;
	inFile.open(filePath.c_str());

	if (!inFile)
	{
		ERROR_MESSAGE(false, "Unable to open file" << filePath);
	}
	else
	{
		string line;
		char buffer[256];
		while (!inFile.eof())
		{
			inFile.getline(buffer, 100);
			if (buffer[0] != ';')
			{
				stringstream ss;
				ss << buffer;
				line = ss.str();
				token = split(line, ">");
				graph.early = utilities::string::trim(token.key);
				token = split(token.value, ">");
				graph.mid = utilities::string::trim(token.key);
				graph.late = utilities::string::trim(token.value);
			}
		}
		inFile.close();
	}
	DEBUG_MESSAGE(utilities::LogLevel_Info, "BuildOrder: Transition config file " << filePath << " loaded");
	return graph;
}

vector<bats::CoreUnit> BuildOrderFileReader::getUnitList(){
	return mCoreUnitsList;
}

vector<UnitType> BuildOrderFileReader::readBuildOrder(string phase, string fileName, vector<UnitType> &buildOrder){
	//string filename = getFilename("buildorder\\" + phase + "\\");
	//vector<UnitType> buildOrder;
	DEBUG_MESSAGE(utilities::LogLevel_Info, "BuildOrder: changed to phase " << phase);
	//Read buildorder file
	ifstream inFile;

	string filePath = config::build_order::DIR + phase + "\\" + fileName + BUILD_ORDER_EXT;

	inFile.open(filePath.c_str());
	string type = "none";
	if (!inFile)
	{
		ERROR_MESSAGE(false, "Unable to open file " << filePath);
	}
	else
	{
		string line;
		char buffer[256];
				
		while (!inFile.eof())
		{
			inFile.getline(buffer, 100);
			if (buffer[0] != ';')
			{
				stringstream ss;
				ss << buffer;
				line = ss.str();
				if (line == "<build-order>")
					type = "build-order";
				else if(line == "<units>")
					type = "units";
				else if(line == "<must-have>")
					type = "must-have";
				else if(type == "build-order")
					addBuildingType(line, buildOrder);				
				else if(type == "units")			
					addUnitType(line, mCoreUnitsList, false);
				else if(type == "must-have")
					addUnitType(line, mCoreUnitsList, true);
			}
		}
		inFile.close();
	}

	DEBUG_MESSAGE(utilities::LogLevel_Info, "Build order file " << filePath << " loaded");
	return buildOrder;
}

void BuildOrderFileReader::addUnitType(string line, vector<bats::CoreUnit> &coreUnits, bool mustHave){
	if (line == "") return;	
	Tokens token;			
	string quantity;
	token = split(line, " ");
	line = token.value;
	quantity = token.key;
	// remove the percentage symbol
	if(!mustHave)
	quantity.erase(quantity.length()-2, quantity.length()-1);

	//Replace all _ with whitespaces, or they wont match
	replace(line);

	CoreUnit unit;
	UnitType unitType = UnitTypes::getUnitType(line);
	if (unitType != UnitTypes::Unknown) {
		//Compose the unit
		unit.mustHave = mustHave;
		unit.quantity = toInt(quantity);
		unit.unit = unitType;
		//Add unit to list | vector
		coreUnits.push_back(unit);
	} 
	//No UnitType match found
	else {
		ERROR_MESSAGE(false, "No matching UnitType found for " << line);
	}
}

void BuildOrderFileReader::addBuildingType(string line, vector<UnitType> &buildOrder){
	if (line == "") return;

	//Replace all _ with whitespaces, or they wont match
	replace(line);

	UnitType unitType = BWAPI::UnitTypes::getUnitType(line);
	if (unitType != UnitTypes::Unknown) {
		buildOrder.push_back(unitType);
	} else {
		ERROR_MESSAGE(false, "No matching buliding found for " << line);
	}
}