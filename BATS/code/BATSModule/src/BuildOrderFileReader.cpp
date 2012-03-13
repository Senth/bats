#include "BuildOrderFileReader.h"
#include "BTHAIModule/Source/ExplorationManager.h"
#include "BuildPlanner.h"
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
	ss << getScriptPath();
	ss << "buildorder\\";
	ss << fileName;
	string filePath = ss.str();
	Tokens token;
	inFile.open(filePath.c_str());

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
					type = "none";
				else if(type == "build-order")
					addUnitType(line, buildOrder);				
			}
		}
		inFile.close();
	}
	//Broodwar->drawTextScreen(15,50,"Build order file %s loaded", filePath.c_str());
	Broodwar->printf("Build order file %s loaded", filePath.c_str());
	return buildOrder;
}

void BuildOrderFileReader::addUnitType(string line, vector<UnitType> &buildOrder){
	if (line == "") return;

	//Replace all _ with whitespaces, or they wont match
	replace(line);
	
	for(set<UnitType>::const_iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end();i++)
	{
		if ((*i).getName() == line)
		{
			buildOrder.push_back((*i));
			return;
		}
	}

	//No UnitType match found
	Broodwar->printf("Error: No matching UnitType found for %s", line.c_str());
}