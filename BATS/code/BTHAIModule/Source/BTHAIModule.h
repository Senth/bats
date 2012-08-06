#pragma once

// Forward declarations
class Statistics;
class AIloop;

#include <BWTA.h>
#include <windows.h>
#include <time.h>
#include "BWTAExtern.h"

#define TOURNAMENT_NAME "AIIDE 2011"
#define SPONSORS "the BWAPI Project Team"
#define MINIMUM_COMMAND_OPTIMIZATION 1

class BTHAITournamentModule : public BWAPI::TournamentModule
{
  virtual bool onAction(int actionType, void *parameter = NULL);
  virtual void onFirstAdvertisement();
};

DWORD WINAPI AnalyzeThread();

/** This class contains the main game loop and all events that is broadcasted from the Starcraft engine
 * using BWAPI. See the BWAPI documentation for more info. 
 *
 * Author: Contained in BWAPI 3.0.3
 * Modified: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BTHAIModule : public BWAPI::AIModule
{
private:
	virtual void gameStopped();

protected:
	bool running;
	bool profile;
	int speed;
	Statistics* statistics;
	AIloop* loop;

public:
	/**
	 * Default constructor to set variables if forgotten to set them manually
	 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
	 */
	BTHAIModule();

	/**
	 * Virtual descructor to call destructors in derived classes.
	 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
	 */
	virtual ~BTHAIModule();
};
