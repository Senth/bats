#pragma once

#include "PathObj.h"
#include "cthread.h"
#include <BWAPI/TilePosition.h>


/** This class is used to find a path between two tiles in the game world. Currently it uses the 
 * A-star implementation in BWTA, but it can easily be changed to another algorithm if needed.
 *
 * The pathfinder is threaded, so agents have to request a path that is put in a queue. Agents have to
 * check the isReady() method to find out when the path finding is finished.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @todo doxygen
 * @todo names
 * @todo const and references
 */
class Pathfinder : public CThread {

private:
	Pathfinder();
	static Pathfinder* instance;
	static bool instanceFlag;
	bool running;
	bool lock;
	bool end;
	

	std::vector<PathObj*> pathObj;

	PathObj* getPathObj(BWAPI::TilePosition start, BWAPI::TilePosition end);

	bool isRunning();

	void setThreadEnded();
	bool isThreadDead();

public:
	/** Destructor */
	~Pathfinder();

	/** Returns the instance of the class. */
	static Pathfinder* getInstance();

	/** Returns the ground distance between two positions. */
	int getDistance(BWAPI::TilePosition start, BWAPI::TilePosition end);

	void requestPath(BWAPI::TilePosition start, BWAPI::TilePosition end);

	bool isReady(BWAPI::TilePosition start, BWAPI::TilePosition end);

	/** Returns the path between two positions. */
	std::vector<BWAPI::TilePosition> getPath(BWAPI::TilePosition start, BWAPI::TilePosition end);

	/** Stops the pathfinder thread. */
	void stop();

	/** Thread update method. */
	unsigned long Process (void* parameter);
};