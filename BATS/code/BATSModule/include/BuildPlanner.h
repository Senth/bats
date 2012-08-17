#pragma once

#include <BWAPI.h>
#include "UnitCreator.h"

// Namespace for the project
namespace bats {

/**
 * Different types in the build queue
 */
enum BuildTypes {
	BuildType_Structure,
	BuildType_Upgrade,
	BuildType_Tech,
	
	BuildType_Lim
};

/**
 * Queue item for the build order, can be a structure, tech upgrade, or regular upgrade.
 */
struct BuildItem {
	BuildTypes type;
	BWAPI::TechType tech; /**< None when not tech type */
	BWAPI::UnitType structure; /**< None when not structure */
	BWAPI::UpgradeType upgrade; /**< None when not upgrade */
	int assignedFrame;
	int assignedWorkerId; /**< Not used for upgrades */

	/**
	 * Constructor for tech upgrade. Structure and upgrade will be set to None.
	 * @param techType type of tech for this build item
	 */
	BuildItem(const BWAPI::TechType& techType) :
		type(BuildType_Tech), tech(techType), structure(BWAPI::UnitTypes::None),
		upgrade(BWAPI::UpgradeTypes::None), assignedFrame(0), assignedWorkerId(-1) {
	}

	/**
	 * Constructor for structures. tech and upgrade will be set to None.
	 * @param structureType type of structure to build
	 */
	BuildItem(const BWAPI::UnitType& structureType) :
		type(BuildType_Structure), tech(BWAPI::TechTypes::None), structure(structureType),
		upgrade(BWAPI::UpgradeTypes::None), assignedFrame(0), assignedWorkerId(-1) {
	}

	/**
	 * Constructor for regular upgrade. Tech and structure will be set to None.
	 * @param upgradeType type of regular upgrade for this build item
	 */
	BuildItem(const BWAPI::UpgradeType& upgradeType) :
		type(BuildType_Upgrade), tech(BWAPI::TechTypes::None), structure(BWAPI::UnitTypes::None),
		upgrade(upgradeType), assignedFrame(0), assignedWorkerId(-1) {
	}
};
struct TransitionGraph {
	std::string early,mid,late;	
};

/**
 * Manages build orders in three phases: early, mid, and late game.
 * Handles user command "transition"
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 */
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
	bool executeOrder(const BuildItem& type);
	bool shallBuildSupplyDepot() const;
	std::string format(const BuildItem& type) const;
	bool hasResourcesLeft() const;
	int mineralsNearby(BWAPI::TilePosition center) const;

	std::vector<BuildItem> mBuildOrder;
	std::vector<BuildItem> mBuildQueue;
	int mLastCommandCenter;	/**< @deprecated, don't use */
	int mLastCallFrame;
	std::vector<bats::CoreUnit> mCoreUnitsList;
	std::string mCurrentPhase;	

	static BuildPlanner* instance;
};
}