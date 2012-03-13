#pragma once

#include <BWAPI.h>
// Namespace for the project
namespace bats {

/**
 * Manages build orders in three phases: early, mid, and late game.
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

private:
	std::vector<BWAPI::UnitType> buildOrder;
	std::vector<BuildQueueItem> buildQueue;
	int lastCommandCenter;
	
	int lastCallFrame;
protected:
	BuildPlanner();
	void lock(int buildOrderIndex, int unitId);
	bool executeOrder(BWAPI::UnitType type);
	bool shallBuildSupplyDepot();
	std::string format(BWAPI::UnitType type);
	bool hasResourcesLeft();
	int mineralsNearby(BWAPI::TilePosition center);	

	static BuildPlanner* instance;

public:
	// transition graph used for maintaining the path taken to current phase
	TransitionGraph mtransitionGraph;
	/** Destructor. */
	~BuildPlanner();
	static std::string mCurrentPhase;
	/** Returns the instance to the BuildPlanner that is currently used. */
	static BuildPlanner* getInstance();

	/** Called on user command to switch phase. */	
	void switchToPhase(std::string fileName);

	/** Returns the number of units of the specified type currently being produced. */
	int noInProduction(BWAPI::UnitType type);

	/** Called each update to issue orders. */
	void computeActions();

	/** Notifies that an own unit has been destroyed. */
	void buildingDestroyed(BWAPI::Unit* building);

	/** When a request to construct a new building is issued, no construction are
	 * allowed until the worker has moved to the buildspot and started constructing
	 * the building. This is to avoid that the needed resources are not used up by
	 * other build orders. During this time the BuildPlanner is locked, and new 
	 * construction can only be done when unlock has been called. */
	void unlock(BWAPI::UnitType type);

	/** Removes a building from the buildorder. */
	void remove(BWAPI::UnitType type);

	/** Called when a worker that is constructing a building is destroyed. */
	void handleWorkerDestroyed(BWAPI::UnitType type, int workerID);

	/** Sets that a new command center has been built. */
	void commandCenterBuilt();

	/** Shows some debug info on screen. */
	void printInfo();

	/** Is called when no buildspot has been found for the specified type. Gives each buildplanner
	 * an opportunity to handle it. */
	void handleNoBuildspotFound(BWAPI::UnitType toBuild);

	/** Checks if more supply buildings are needed. */
	bool shallBuildSupply();

	/** Checks if a supply is under construction. */
	bool supplyBeingBuilt();

	/** Returns true if next in buildorder is of the specified type. Returns false if
	 * buildorder is empty. */
	bool nextIsOfType(BWAPI::UnitType type);

	/** Returns true if buildorder contains a unit of the specified type. */
	bool containsType(BWAPI::UnitType type);

	/** Adds a building to the buildorder queue. */
	void addBuilding(BWAPI::UnitType type);

	/** Adds a building first in the buildorder queue. */
	void addBuildingFirst(BWAPI::UnitType type);

	/** Requests to expand the base. */
	void expand(BWAPI::UnitType commandCenterUnit);

	/** Adds a refinery to the buildorder list. */
	void addRefinery();

	/** Checks if the specified BWAPI::TilePosition is covered by a detector buildings sight radius. */
	static bool coveredByDetector(BWAPI::TilePosition pos);

	/** Morphs a Zerg drone to a building. */
	bool executeMorph(BWAPI::UnitType target, BWAPI::UnitType evolved);

	/** Returns true if the player is Terran. */
	static bool isTerran();

	/** Returns true if the player is Protoss. */
	static bool isProtoss();

	/** Returns true if the player is Zerg. */
	static bool isZerg();
};
}