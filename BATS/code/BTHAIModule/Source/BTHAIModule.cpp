#include "BTHAIModule.h"
#include "AgentManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "CoverMap.h"
#include "Pathfinder.h"
#include "UpgradesPlanner.h"
#include "ResourceManager.h"
//#include "AIloop.h"
#include "Statistics.h"
#include "Profiler.h"
#include "Config.h"
#include <Shlwapi.h>

using namespace BWAPI;
using namespace std;

bool analyzed;
bool analysis_just_finished;
bool leader = false;

BTHAIModule::BTHAIModule() : BWAPI::AIModule() {
	statistics = NULL;
	//loop = NULL;

	running = false;
	profile = false;
	speed = 8;
}

BTHAIModule::~BTHAIModule() {
	delete statistics;
	//delete loop;
}

void BTHAIModule::onStart() 
{
	Profiler::getInstance()->start("OnInit");

	//Broodwar->printf("The map is %s, a %d player map", Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
	
	//Needed for BWAPI to work
	Broodwar->enableFlag(Flag::UserInput);
	//Set max speed
	speed = 8; //10
	Broodwar->setLocalSpeed(speed);

	//Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);
	
	//Analyze map using BWTA
	BWTA::readMap();
	analyzed=false;
	analysis_just_finished=false;
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL); //Threaded version
	AnalyzeThread();

	profile = false;

	//Init our singleton agents
	CoverMap::getInstance();
	bats::BuildPlanner::getInstance();
	UpgradesPlanner::getInstance();
	ResourceManager::getInstance();
	Pathfinder::getInstance();
	//loop = new AIloop();
	//loop->setDebugMode(1);

	if (Broodwar->isReplay()) 
	{
		Broodwar->printf("The following players are in this replay:");
		for(std::set<Player*>::iterator p=Broodwar->getPlayers().begin();p!=Broodwar->getPlayers().end();p++)
		{
			if (!(*p)->getUnits().empty() && !(*p)->isNeutral())
			{
				Broodwar->printf("%s, playing as a %s",(*p)->getName().c_str(),(*p)->getRace().getName().c_str());
			}
		}
	}
	
    //Add the units we have from start to agent manager
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++) 
	{
		AgentManager::getInstance()->addAgent(*i);
	}

	//Broodwar->printf("BTHAI %s (%s)", VERSION.c_str(), Broodwar->self()->getRace().getName().c_str());

	running = true;

	Profiler::getInstance()->end("OnInit");
}

void BTHAIModule::gameStopped()
{
	//statistics->WriteStatisticsFile(isWinner);
	Pathfinder::getInstance()->stop();
	//delete(statistics);
	Profiler::getInstance()->dumpToFile();
	running = false;
}

void BTHAIModule::onEnd(bool isWinner) 
{
	gameStopped();
}

void BTHAIModule::onFrame() 
{
	Profiler::getInstance()->start("OnFrame");

	if (!running) 
	{
		//Game over. Do nothing.
		return;
	}
	if (!Broodwar->isInGame()) 
	{
		//Not in game. Do nothing.
		gameStopped();
		return;
	}
	if (Broodwar->isReplay()) 
	{
		//Replay. Do nothing.
		return;
	}
	
	//loop->computeActions();
	//loop->show_debug();

	Config::getInstance()->displayBotName();

	Profiler::getInstance()->end("OnFrame");

	if (profile) Profiler::getInstance()->showAll();
}

void BTHAIModule::onSendText(std::string text) 
{
	if (text=="/a") 
	{
		//Commander::getInstance()->forceAttack();
	}
	else if(text=="/p") 
	{
		profile = !profile;
	}
	else if (text=="+") 
	{
		speed -= 4;
		if (speed < 0) 
		{
			speed = 0;
		}
		Broodwar->printf("Speed increased to %d", speed);
		Broodwar->setLocalSpeed(speed);
	}
	else if (text=="++") 
	{
		speed = 0;
		Broodwar->printf("Speed increased to %d", speed);
		Broodwar->setLocalSpeed(speed);
	}
	else if (text=="-") 
	{
		speed += 4;
		Broodwar->printf("Speed decreased to %d", speed);
		Broodwar->setLocalSpeed(speed);
	}
	else if (text=="--") 
	{
		speed = 24;
		Broodwar->printf("Speed decreased to %d", speed);
		Broodwar->setLocalSpeed(speed);
	}
	else if (text=="i") 
	{
		set<Unit*> units = Broodwar->getSelectedUnits();
		if ((int)units.size() > 0) 
		{
			int unitID = (*units.begin())->getID();
			BaseAgent* agent = AgentManager::getInstance()->getAgent(unitID);
			if (agent != NULL) 
			{
				agent->printGraphicDebugInfo();
			}
			else 
			{
				Unit* mUnit = (*units.begin());
				if (mUnit->getType().isNeutral())
				{
					//Neutral unit. Check distance to base.
					BaseAgent* agent = AgentManager::getInstance()->getAgent(UnitTypes::Terran_Command_Center);
					double dist = agent->getUnit()->getDistance(mUnit);
					Broodwar->printf("Distance to base: %d", (int)dist);
				}
			}
			
		}
	}

	else 
	{
		Broodwar->printf("You typed '%s'!",text.c_str());
	}
}

void BTHAIModule::onReceiveText(BWAPI::Player* player, std::string text) 
{
	Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

void BTHAIModule::onPlayerLeft(BWAPI::Player* player) 
{
	if (player->getID() == Broodwar->self()->getID())
	{
		gameStopped();
	}
	Broodwar->sendText("%s left the game.",player->getName().c_str());
}

void BTHAIModule::onNukeDetect(BWAPI::Position target) 
{
	if (target != Positions::Unknown) 
	{
		TilePosition t = TilePosition(target);
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)",t.x(),t.y());
	}
	else
	{
		Broodwar->printf("Nuclear Launch Detected");
	}
}

void BTHAIModule::onUnitDiscover(BWAPI::Unit* unit) 
{
	
}

void BTHAIModule::onUnitEvade(BWAPI::Unit* unit) 
{
	
}

void BTHAIModule::onUnitShow(BWAPI::Unit* unit) 
{
	//if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

	//if (unit->getPlayer()->getID() != Broodwar->self()->getID()) 
	//{
	//	if (!unit->getPlayer()->isNeutral() && !unit->getPlayer()->isAlly(Broodwar->self()))
	//	{
	//		ExplorationManager::getInstance()->addSpottedUnit(unit);
	//	}
	//}
}

void BTHAIModule::onUnitHide(BWAPI::Unit* unit) 
{
	
}

void BTHAIModule::onUnitCreate(BWAPI::Unit* unit)
{
	if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

	//loop->addUnit(unit);
}

void BTHAIModule::onUnitDestroy(BWAPI::Unit* unit) 
{
	if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

	//loop->unitDestroyed(unit);
}

void BTHAIModule::onUnitMorph(BWAPI::Unit* unit) 
{
	//if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

	//if (unit->getPlayer()->getID() == Broodwar->self()->getID()) 
	//{
	//	if (bats::BuildPlanner::isZerg())
	//	{
	//		loop->morphUnit(unit);
	//	}
	//	else
	//	{
	//		loop->addUnit(unit);
	//	}
	//}
}

void BTHAIModule::onUnitRenegade(BWAPI::Unit* unit) 
{
	
}

void BTHAIModule::onSaveGame(std::string gameName) 
{
	Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

DWORD WINAPI AnalyzeThread()
{
	BWTA::analyze();
	
	analyzed = true;
	analysis_just_finished = true;
	return 0;
}

// Use some existing Battle.net filters
const wchar_t *pszBadWords[] =
{ 
  L"asshole",L"bitch",L"clit",L"cock",L"cunt",L"dick",L"dildo",L"faggot",L"fuck",L"gook",L"masturbat",L"nigga",L"nigger",L"penis",L"pussy",L"shit",L"slut",L"whore",NULL
};

// as well as the Battle.net swear word filter algorithm
const wchar_t szBadWordCharacters[] = { '!', '@', '#', '$', '%', '&' };
void BadWordFilter(wchar_t *pszString)
{
  // Iterate each badword
  for ( int f = 0; pszBadWords[f]; ++f )
  {
    // Find badword
    wchar_t* pszMatch = StrStrI(pszString, pszBadWords[f]);
    if ( !pszMatch )
      continue; // continue if badword not found

    // iterate characters in badword
    wchar_t cLast = 0;
    for ( int i = 0; pszBadWords[f][i]; ++i )
    {
      // make the character compatible with our replacements
      int val = pszBadWords[f][i] & 7;
      if ( val >= sizeof(szBadWordCharacters) )
        val = 0;

      // increment the replacement if it's the same as our last one, reset to 0 if it's out of bounds
      if ( cLast == szBadWordCharacters[val] && ++val == sizeof(szBadWordCharacters) )
        val = 0;

      // apply our change to the original string and save the last character used
      pszMatch[i] = szBadWordCharacters[val];
      cLast       = szBadWordCharacters[val];
    }
  }
}

bool BTHAITournamentModule::onAction(int actionType, void *parameter)
{
  switch ( actionType )
  {
  case Tournament::SendText:
  case Tournament::Printf:
    // Call our bad word filter and allow the AI module to send text
    BadWordFilter((wchar_t*)parameter);
    return true;
  case Tournament::EnableFlag:
    switch ( *(int*)parameter )
    {
    case Flag::CompleteMapInformation:
    case Flag::UserInput:
      // Disallow these two flags
      return false;
    }
    // Allow other flags if we add more that don't affect gameplay specifically
    return true;
  case Tournament::LeaveGame:
  case Tournament::PauseGame:
  case Tournament::RestartGame:
  case Tournament::ResumeGame:
  case Tournament::SetFrameSkip:
  case Tournament::SetGUI:
  case Tournament::SetLocalSpeed:
  case Tournament::SetMap:
    return false; // Disallow these actions
  case Tournament::ChangeRace:
  case Tournament::SetLatCom:
  case Tournament::SetTextSize:
    return true; // Allow these actions
  case Tournament::SetCommandOptimizationLevel:
    return *(int*)parameter > MINIMUM_COMMAND_OPTIMIZATION; // Set a minimum command optimization level 
                                                            // to reduce APM with no action loss
  default:
    break;
  }
  return true;
}

void BTHAITournamentModule::onFirstAdvertisement()
{
  leader = true;
  Broodwar->sendText("Welcome to " TOURNAMENT_NAME "!");
  Broodwar->sendText("Brought to you by " SPONSORS ".");
}
