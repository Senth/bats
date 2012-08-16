#pragma once

#include <BWAPI.h>
#include "UnitCreator.h"

// Namespace for the project
namespace bats {

/**
 * Manages build orders in three phases: early, mid, and late game.
 * Handles user command "transition"
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 */
struct BuildQueueItem {
	BWAPI::UnitType toBuild;
	int assignedFrame;
	int assignedWorkerId;
};
struct TransitionGraph {
	std::string early,mid,late;	
};

class BuildPlanner {
public:
	// transition graph used for maintaining the path taken to current phase
	TransitionGraph mtransitionGraph;
	/** Destructor. */
	~BuildPlanner();
	
	/** Returns the instance to the BuildPlanner that is currently used. */
	static BuildPlanner* getInstance();

	/**
	 * Returns the number of structures in the queue, i.e. not in production
	 * @return number of structures left to build
	 */
	size_t getQueueCount() const;

	/**
	 * Used to get the unit list created from the BuildOrderFileReader instance
	 */
	std::vector<bats::CoreUnit> getUnitList() const;

	/**
	 * Returns the current phase of the Build Planner, i.e. early, mid, or late game.
	 * @return current phase of the Build Planner, possible values are "early", "mid", or "late".
	 */
	const std::string& getCurrentPhase() const;

	/**
	 * Checks if we can transition, this essentially checks if we're not the late game, as
	 * we can't transition any more when we're in the late game.
	 */
	bool canTransition() const;

	/** Called on user command to switch phase. */
	void switchToPhase(std::string fileName);

	/** Returns the number of units of the specified type currently being produced. */
	int countInProduction(BWAPI::UnitType type) const;

	/** Called each update to issue orders. */
	void computeActions();

	/** Notifies that an own unit has been destroyed. */
	void buildingDestroyed(BWAPI::Unit* building);

	/** When a request to construct a new building is issued, no construction are
	 * allowed until the worker has moved to the build spot and started constructing
	 * the building. This is to avoid that the needed resources are not used up by
	 * other build orders. During this time the BuildPlanner is locked, and new 
	 * construction can only be done when unlock has been called. */
	void unlock(BWAPI::UnitType type);

	/** Removes a building from the build order. */
	void remove(BWAPI::UnitType type);

	/** Called when a worker that is constructing a building is destroyed. */
	void handleWorkerDestroyed(BWAPI::UnitType type, int workerID);

	/** Sets that a new command center has been built. */
	void commandCenterBuilt();

	/** Shows some debug info on screen. */
	void printGraphicDebugInfo() const;

	/** Is called when no build spot has been found for the specified type. Gives each build planner
	 * an opportunity to handle it. */
	void handleNoBuildspotFound(BWAPI::UnitType toBuild);

	/** Checks if more supply buildings are needed. */
	bool shallBuildSupply() const;

	/** Checks if a supply is under construction. */
	bool supplyBeingBuilt() const;

	/** Returns true if next in build order is of the specified type. Returns false if
	 * build order is empty. */
	bool nextIsOfType(BWAPI::UnitType type) const;

	/** Returns true if build order contains a unit of the specified type. */
	bool containsType(BWAPI::UnitType type) const;

	/** Adds a building to the build order queue. */
	void addBuilding(BWAPI::UnitType type);

	/** Adds a building first in the build order queue. */
	void addBuildingFirst(BWAPI::UnitType type);

	/** Requests to expand the base. */
	void expand();

	/** Check if expansion is available. */
	bool isExpansionAvailable() const;

	/** Adds a refinery to the build order list. */
	void addRefinery();

	/** Checks if the specified BWAPI::TilePosition is covered by a detector buildings sight radius.
	 * @deprecated should not be here */
	static bool coveredByDetector(BWAPI::TilePosition pos);

	/** Morphs a Zerg drone to a building. */
	bool executeMorph(BWAPI::UnitType target, BWAPI::UnitType evolved);

	/** Returns true if the player is Terran.
	 * @deprecated should not be here */
	static bool isTerran();

	/** Returns true if the player is Protoss.
	 * @deprecated should not be here */
	static bool isProtoss();

	/** Returns true if the player is Zerg. 
	 * @deprecated should not be here */
	static bool isZerg();

private:
	BuildPlanner();
	void lock(int buildOrderIndex, int unitId);
	bool executeOrder(BWAPI::UnitType type);
	bool shallBuildSupplyDepot() const;
	std::string format(BWAPI::UnitType type) const;
	bool hasResourcesLeft() const;
	int mineralsNearby(BWAPI::TilePosition center) const;

	std::vector<BWAPI::UnitType> mBuildOrder;
	std::vector<BuildQueueItem> mBuildQueue;
	int mLastCommandCenter;	/**< @deprecated, don't use */
	int mLastCallFrame;
	std::vector<bats::CoreUnit> mCoreUnitsList;
	std::string mCurrentPhase;	

	static BuildPlanner* instance;
};
}