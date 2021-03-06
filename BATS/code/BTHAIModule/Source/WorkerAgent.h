#pragma once

#include "UnitAgent.h"
#include <map>

// Forward declarations
class CoverMap;

/** The WorkerAgent class handles all tasks that a worker, for example a Terran SCV, can perform. The tasks
 * involves gathering minerals and gas, move to a selected buildspot and construct the specified building,
 * and if Terran SCV, repair a building or tank.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 * @todo doxygen
 * @todo variable names
 * @todo const and references
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
	void printGraphicDebugInfo() const;

	/** Set the state of the worker. I.e. what does it do right now. 
	 * Should only be set if the worker is getting a task not through the functions in this class. Then it is automatic. */
	void setState(States state);

	/** Returns the current state of the worker. */
	States getState() const;

	/** Returns true if the Worker agent can create units of the specified type. */
	bool canBuild(const BWAPI::UnitType& type) const;

	/** Assigns the agent to repair a building or tank. */
	bool assignToRepair(const BWAPI::Unit* unit);

	/** Assigns the unit to construct a building of the specified type. */
	bool assignToBuild(const BWAPI::UnitType& type);

	/** Assigns the agent to continue building a non-finished building. */
	bool assignToFinishBuild(const BWAPI::Unit* building);

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
	bool isConstructing(const BWAPI::UnitType& type = BWAPI::UnitTypes::None) const;

protected:
	virtual std::string getDebugString() const;

private:
	__declspec(deprecated) void handleKitingWorker();
	__declspec(deprecated) BWAPI::Unit* getEnemyUnit();
	__declspec(deprecated) BWAPI::Unit* getEnemyBuilding();
	__declspec(deprecated) BWAPI::Unit* getEnemyWorker();
	bool buildSpotExplored() const;
	bool areaFree() const;

	/**
	 * If the unit has completed building the assigned building.
	 * @return true if the worker completed building the assigned building.
	 */
	bool hasCompletedBuilding() const;

	/**
	 * Finds a close structures and mechanical units to repair, prioritizes the unit with
	 * least % health left. Does not return a unit that is already being repaired.
	 * @return unit that needs a repairer
	 */
	const BWAPI::Unit* findRepairUnit() const;

	/**
	 * Checks if the unit already has the maximum number of repairers
	 * @param unit the unit to check
	 * @return true if more repair workers can be added to repair this unit
	 */
	static bool canMoreScvsRepairThisUnit(const BWAPI::Unit* unit);

	/**
	 * Decreases the number of repair SCVs this unit has
	 * @param unit the unit to decrease the number of repair SCVs
	 */
	static void decreaseRepairScvs(const BWAPI::Unit* unit);

	/**
	 * Increases the number of repair SCVs this unit has
	 * @param unit the unit to increase the number of repair SCVs
	 */
	static void increaseRepairScvs(const BWAPI::Unit* unit);

	/**
	 * Tests if the repairing is of the current unit it is repairing.
	 * If the repairing is done it will automatically remove the unit and decrease
	 * the number of scvs repairing it.
	 * @return true if either the repairing is done or this unit isn't repairing anything
	 * at the moment
	 */
	bool testRepairDone();

	States mCurrentState;
	BWAPI::UnitType mToBuild;
	BWAPI::TilePosition mBuildSpot;
	const BWAPI::Unit* mRepairUnit;

	static std::map<const BWAPI::Unit*, int> msRepairUnits;
	static CoverMap* msCoverMap;
};