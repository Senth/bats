#ifndef __AGENTMANAGER_H__
#define __AGENTMANAGER_H__

#include <BWAPI.h>
#include "BaseAgent.h"
#include "cthread.h"

/** The AgentManager class is a container that holds a list of all active agents in the game. Each unit, worker, building or
 * or addon is assigned to an agent. See the MainAgents, StructureAgents and UnitAgents folders for detailed information
 * about each specific type of agent.
 *
 * The AgentManager is implemented as a singleton class. Each class that needs to access AgentManager can request an instance,
 * and all classes shares the same AgentManager instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */

class AgentManager {

protected:
	std::vector<BaseAgent*> agents;
	int lastCallFrame;
	AgentManager();
	static AgentManager* instance;

	/** Called when an agent of our type is created. I.e. it will first create an agent through
	 * addAgent then call this with the newly created agent. This function can be overridden.
	 * This function shall handle agent specific functionality.
	 * @param newAgent the new agent that has been added.
	 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
	 * @note the agent has already been added to the unit vector.
	 */
	virtual void onAgentCreated(BaseAgent* newAgent);

	/**
	 * Called when an agent of our type has been destroyed. I.e. it will first make sure we
	 * have this agent and then call this function; can be overridden.
	 * @param destroyedAgent the destroyed agent
	 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
	 */
	virtual void onAgentDestroyed(BaseAgent* destroyedAgent);
	 

public:
	static int StartFrame;

	virtual ~AgentManager();

	/** Returns the instance to the AgentManager. */
	static AgentManager* getInstance();

	/** Adds an agent to the container. Is called each time a new
	 * unit is built. */
	void addAgent(BWAPI::Unit* unit);

	/** Removes an agent from the container. Is called each time
	 * a unit is destroyed. The agents are not directly removed, but
	 * set to inactive and are removed during the cleanup. */
	void removeAgent(BWAPI::Unit* unit);

	/** Called when a Zerg Drone is morphed into another unit. */
	void morphDrone(BWAPI::Unit* unit);

	/** Called each update to issue commands from all active agents. */
	void computeActions();

	/** Returns the current number of active worker units. */
	int getNoWorkers();

	/** Returns the current number of active workers gathering minerals. */
	int noMiningWorkers();

	/** Returns the closest free worker from the specified position, or NULL if not found. */
	BaseAgent* findClosestFreeWorker(BWAPI::TilePosition pos);

	/** Checks if any agent has the task to repair this specified agent. */
	bool isAnyAgentRepairingThisAgent(BaseAgent* repairedAgent);

	/** Returns the number of own units of a specific type. */
	int countNoUnits(BWAPI::UnitType type);

	/** Returns the number of units or buildings of the specified type
	 * that currently is in production. */
	int noInProduction(BWAPI::UnitType type);

	/** Returns the number of bases the player has. */
	int countNoBases();

	/** Returns a list of all agents in the container. */
	std::vector<BaseAgent*> getAgents();

	/** Returns the number of agents the exists in the std::vector. */
	int size();

	/** Returns a reference to the agent associated with a specific unit,
	 * or NULL if the unit doesn't exist. */
	BaseAgent* getAgent(int unitID);

	/** Returns the first agent in the list of the specified type, or NULL if not found. */
	BaseAgent* getAgent(BWAPI::UnitType type);

	/** Requests a free Zerg Overlord to move to the specified position. */
	void requestOverlord(BWAPI::TilePosition pos);

	/** Returns the closest agent in the list of the specified type, or NULL if not found. */
	BaseAgent* getClosestAgent(BWAPI::TilePosition pos, BWAPI::UnitType type);

	/** Returns the closest base agent (Terran Command Center, Protoss Nexus), in the list,
	 * or NULL if not found. */
	BaseAgent* getClosestBase(BWAPI::TilePosition pos);

	/** Returns the position of the closest detector unit relative to startPos. Returns
	 * BWAPI::TilePosition(-1, -1) if none was found. */
	BWAPI::TilePosition getClosestDetector(BWAPI::TilePosition startPos);

	/** Checks if there are any units in an area. The unit with id unitID is allowed. */
	bool unitsInArea(BWAPI::TilePosition pos, int tileWidth, int tileHeight, int unitID);

	/** Removes inactive agents from the container. Shouldn't be called too often. */
	void cleanup();
};

#endif
