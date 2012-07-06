#ifndef __WORKERAGENT_H__
#define __WORKERAGENT_H__

#include "UnitAgent.h"

/** The WorkerAgent class handles all tasks that a worker, for example a Terran SCV, can perform. The tasks
 * involves gathering minerals and gas, move to a selected buildspot and construct the specified building,
 * and if Terran SCV, repair a building or tank.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class WorkerAgent : public UnitAgent {
public:
	/**
	 * Different states of the worker
	 */
	enum States {
		GATHER_MINERALS,
		GATHER_GAS,
		FIND_BUILDSPOT, /**< Trying to find a build spot for a requested building */
		MOVE_TO_SPOT, /**< Moving to a found build spot */
		CONSTRUCT, /**< Constructing a building */
		REPAIRING, /**< Repairing (Terran only) */
		ATTACKING /**< Attacking, either in squad or enemy intruder in base */
	};

	/** Constructor. */
	WorkerAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();

	/** Used in debug modes to show a line to the agents' goal. */
	void printGraphicDebugInfo();

	/** Set the state of the worker. I.e. what does it do right now. 
	 * Should only be set if the worker is getting a task not through the functions in this class. Then it is automatic. */
	void setState(States state);

	/** Returns the current state of the worker. */
	States getState() const;

	/** Returns true if the Worker agent can create units of the specified type. */
	bool canBuild(BWAPI::UnitType type) const;

	/** Assigns the agent to repair a building or tank. */
	bool assignToRepair(BWAPI::Unit* building);

	/** Assigns the unit to construct a building of the specified type. */
	bool assignToBuild(BWAPI::UnitType type);

	/** Assigns the agent to continue building a non-finished building. */
	bool assignToFinishBuild(BWAPI::Unit* building);

	/** Returns the state of the agent as text. Good for printouts. */
	std::string getStateAsText() const;

	/** Called when the unit assigned to this agent is destroyed. */
	void destroyed();

	/** Resets a worker to gathering minerals. */
	void reset();

	/** Returns true if this worker is in any of the build states, and is constructing
	 * the specified building.
	 * @param type the type the unit is constructing, when this is set to UnitType::None it
	 * will check if it's constructing anything, defaults to UnitTypes::None
	 * @return true if the worker is building the specified type, or building anything if
	 * type was set to UnitTypes::None. */
	bool isConstructing(BWAPI::UnitType type = BWAPI::UnitTypes::None) const;

private:
	States currentState;

	BWAPI::UnitType toBuild;
	BWAPI::TilePosition buildSpot;
	BWAPI::TilePosition startSpot;
	bool buildSpotExplored() const;
	bool areaFree() const;
	bool isBuilt() const;
	int startBuildFrame;

	void handleKitingWorker();
	BWAPI::Unit* getEnemyUnit();
	BWAPI::Unit* getEnemyBuilding();
	BWAPI::Unit* getEnemyWorker();

	int lastFrame;
};

#endif
