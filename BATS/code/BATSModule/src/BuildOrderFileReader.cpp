#include "BuildOrderFileReader.h"
#include "BTHAIModule/Source/ExplorationManager.h"
#include "BuildPlanner.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <sstream>

using namespace BWAPI;
using namespace std;
using namespace bats;

BuildOrderFileReader::BuildOrderFileReader(){
	//TODO: read the transition config file
}

TransitionGraph BuildOrderFileReader::readTransitionFile(string fileName){
	
	//Read transition file
	ifstream inFile;
	TransitionGraph graph;
	stringstream ss;
	ss << "bwapi-data\\AI\\BATS-data\\";
	ss << "buildorder\\";
	ss << fileName;
	string filePath = ss.str();
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
				graph.early = token.key;			
				token = split(token.value, ">");
				graph.mid = token.key;
				graph.late = token.value;
			}
		}
		inFile.close();
	}
	//Broodwar->drawTextScreen(15,50,"Build order file %s loaded", filePath.c_str());
	Broodwar->printf("Transition config file %s loaded", filePath.c_str());
	return graph;
}

vector<bats::CoreUnit> BuildOrderFileReader::getUnitList(){
	return mCoreUnitsList;
}

vector<UnitType> BuildOrderFileReader::readBuildOrder(string phase, string fileName, vector<UnitType> &buildOrder){
	//string filename = getFilename("buildorder\\" + phase + "\\");
	//vector<UnitType> buildOrder;
	Broodwar->printf("~~~Build Order changed to phase %s", phase);
	//Read buildorder file
	ifstream inFile;

	stringstream ss;
	ss << getScriptPath();
	ss << "buildorder\\" + phase + "\\";	
	ss << fileName + ".txt";	
	string filePath = ss.str();

	inFile.open(filePath.c_str());
	string type = "none";
	if (!inFile)
	{
		Broodwar->printf("Unable to open file %s", filePath.c_str());
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
	//Broodwar->drawTextScreen(15,50,"Build order file %s loaded", filePath.c_str());
	Broodwar->printf("Build order file %s loaded", filePath.c_str());
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
	for(set<UnitType>::const_iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end();i++)
	{
		if ((*i).getName() == line){
			//Compose the unit
			unit.mustHave = mustHave;
			unit.quantity = toInt(quantity);
			unit.unit = (*i);
			//Add unit to list | vector
			coreUnits.push_back(unit);
			return;
		}
	}

	//No UnitType match found
	ERROR_MESSAGE(false, "No matching UnitType found for " << line);
}

void BuildOrderFileReader::addBuildingType(string line, vector<UnitType> &buildOrder){
	if (line == "") return;

	//Replace all _ with whitespaces, or they wont match
	replace(line);
	
	for(set<UnitType>::const_iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end();i++){
		if ((*i).getName() == line){
			buildOrder.push_back((*i));
			return;
		}
	}

	//No Building match found
	Broodwar->printf("Error: No matching Building found for %s", line.c_str());
}