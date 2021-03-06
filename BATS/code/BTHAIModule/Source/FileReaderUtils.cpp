#include "FileReaderUtils.h"
#include "BatsModule/include/BuildPlanner.h"
#include "Utilities/Logger.h"
#include "Config.h"
#include <fstream>
#include <sstream>

using namespace BWAPI;
using namespace std;

FileReaderUtils::FileReaderUtils()
{
	
}

string FileReaderUtils::getFilename(string subpath)
{
	string filename = "";

	//if (bats::BuildPlanner::isProtoss())
	//{
	//	if (Broodwar->enemy()->getRace() == Races::Protoss)
	//	{
	//		filename = "PvP.txt";
	//		if (!fileExists(subpath, filename)) filename = "PvX.txt";
	//	}
	//	else if (Broodwar->enemy()->getRace() == Races::Terran)
	//	{
	//		filename = "PvT.txt";
	//		if (!fileExists(subpath, filename)) filename = "PvX.txt";
	//	}
	//	else if (Broodwar->enemy()->getRace() == Races::Zerg)
	//	{
	//		filename = "PvZ.txt";
	//		if (!fileExists(subpath, filename)) filename = "PvX.txt";
	//	}
	//}
	//else if (bats::BuildPlanner::isTerran())
	//{
	//	if (Broodwar->enemy()->getRace() == Races::Protoss)
	//	{
	//		filename = "TvP.txt";
	//		if (!fileExists(subpath, filename)) filename = "TvX.txt";
	//	}
	//	else if (Broodwar->enemy()->getRace() == Races::Terran)
	//	{
	//		filename = "TvT.txt";
	//		if (!fileExists(subpath, filename)) filename = "TvX.txt";
	//	}
	//	else if (Broodwar->enemy()->getRace() == Races::Zerg)
	//	{
	//		filename = "TvZ.txt";
	//		if (!fileExists(subpath, filename)) filename = "TvX.txt";
	//	}
	//}
	//else if (bats::BuildPlanner::isZerg())
	//{
	//	if (Broodwar->enemy()->getRace() == Races::Protoss)
	//	{
	//		filename = "ZvP.txt";
	//		if (!fileExists(subpath, filename)) filename = "ZvX.txt";
	//	}
	//	else if (Broodwar->enemy()->getRace() == Races::Terran)
	//	{
	//		filename = "ZvT.txt";
	//		if (!fileExists(subpath, filename)) filename = "ZvX.txt";
	//	}
	//	else if (Broodwar->enemy()->getRace() == Races::Zerg)
	//	{
	//		filename = "ZvZ.txt";
	//		if (!fileExists(subpath, filename)) filename = "ZvX.txt";
	//	}
	//}
	//else
	//{

	//}
	return filename;
}

string FileReaderUtils::getScriptPath()
{
	return Config::getInstance()->getScriptPath();
}

bool FileReaderUtils::fileExists(string subpath, string filename)
{
	ifstream inFile;

	stringstream ss;
	ss << getScriptPath();
	ss << subpath;
	ss << "\\";
	ss << filename;
	string filePath = ss.str();

	inFile.open(filePath.c_str());

	if (!inFile)
	{
		return false;
	}
	else
	{
		inFile.close();
		return true;
	}
}

UnitType FileReaderUtils::getUnitType(string line)
{
	if (line == "") return UnitTypes::Unknown;

	//Replace all _ with whitespaces, or they wont match
	replace(line);
	
	for(set<UnitType>::const_iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end();i++)
	{
		if ((*i).getName() == line)
		{
			return (*i);
		}
	}

	//No UnitType match found
	ERROR_MESSAGE(false, "No matching UnitType found for " << line);
	return UnitTypes::Unknown;
}

UpgradeType FileReaderUtils::getUpgradeType(string line)
{
	if (line == "") return UpgradeTypes::Unknown;

	//Replace all _ with whitespaces, or they wont match
	replace(line);
	
	for(set<UpgradeType>::const_iterator i=UpgradeTypes::allUpgradeTypes().begin();i!=UpgradeTypes::allUpgradeTypes().end();i++)
	{
		if ((*i).getName() == line)
		{
			return (*i);
		}
	}

	//No UnitType match found
	return UpgradeTypes::Unknown;
}

TechType FileReaderUtils::getTechType(string line)
{
	if (line == "") return TechTypes::Unknown;

	//Replace all _ with whitespaces, or they wont match
	replace(line);
	
	for(set<TechType>::const_iterator i=TechTypes::allTechTypes().begin();i!=TechTypes::allTechTypes().end();i++)
	{
		if ((*i).getName() == line)
		{
			return (*i);
		}
	}

	//No UnitType match found
	ERROR_MESSAGE(false, "No matching TechType found for " << line);
	return TechTypes::Unknown;
}

void FileReaderUtils::replace(string &line)
{
	int usIndex = line.find("_");
	while (usIndex != string::npos)
	{
		line.replace(usIndex, 1, " ");
		usIndex = line.find("_");
	}
}

int FileReaderUtils::toInt(string &str)
{
	stringstream ss(str);
	int n;
	ss >> n;
	return n;
}

Tokens FileReaderUtils::split(string line, string delimiter)
{
	Tokens tokens;
	tokens.key = "";
	tokens.value = "";

	int eqIndex = line.find(delimiter);
	if (eqIndex != string::npos)
	{
		tokens.key = line.substr(0, eqIndex);
		tokens.value = line.substr(eqIndex + 1, line.length());
	}
	return tokens;
}
