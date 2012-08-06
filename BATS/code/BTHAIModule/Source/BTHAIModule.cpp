#include "BTHAIModule.h"
#include "AgentManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "CoverMap.h"
#include "Pathfinder.h"
#include "UpgradesPlanner.h"
#include "ResourceManager.h"
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

void BTHAIModule::gameStopped()
{
	//statistics->WriteStatisticsFile(isWinner);
	Pathfinder::getInstance()->stop();
	//delete(statistics);
	Profiler::getInstance()->dumpToFile();
	running = false;
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
