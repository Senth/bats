#pragma once

#include <BWAPI.h>
#include "UnitCreator.h"

// Forward declarations
class ResourceManager;
class CoverMap;
class AgentManager;

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
	int upgradeLevel;
	int assignedFrame;
	int assignedBuilderId; /**< Not used for upgrades */

	/**
	 * Constructor for tech upgrade. Structure and upgrade will be set to None.
	 * @param techType type of tech for this build item
	 */
	BuildItem(const BWAPI::TechType& techType) :
		type(BuildType_Tech), tech(techType), structure(BWAPI::UnitTypes::None),
		upgrade(BWAPI::UpgradeTypes::None), upgradeLevel(0),
		assignedFrame(0), assignedBuilderId(-1) {
	}

	/**
	 * Constructor for structures. tech and upgrade will be set to None.
	 * @param structureType type of structure to build
	 */
	BuildItem(const BWAPI::UnitType& structureType) :
		type(BuildType_Structure), tech(BWAPI::TechTypes::None), structure(structureType),
		upgrade(BWAPI::UpgradeTypes::None), upgradeLevel(0),
		assignedFrame(0), assignedBuilderId(-1) {
	}

	/**
	 * Constructor for regular upgrade. Tech and structure will be set to None.
	 * @param upgradeType type of regular upgrade for this build item
	 */
	BuildItem(const BWAPI::UpgradeType& upgradeType) :
		type(BuildType_Upgrade), tech(BWAPI::TechTypes::None), structure(BWAPI::UnitTypes::None),
		upgrade(upgradeType), upgradeLevel(1),
		assignedFrame(0), assignedBuilderId(-1) {
	}

	/**
	 * Equality operator
	 * @param rhs the right hand side item
	 * @return true if both items are the same type (i.e. same structure/tech/upgrade type)
	 * and additionally for upgrades same upgradeLevel
	 */
	bool operator==(const BuildItem& rhs) const {
		if (type != rhs.type) {
			return false;
		}

		switch (type) {
			case BuildType_Upgrade:
				return upgrade == rhs.upgrade && upgradeLevel == rhs.upgradeLevel;

			case BuildType_Tech:
				return tech == rhs.tech;

			case BuildType_Structure:
				return structure == rhs.structure;

			default:
				return false;
		}
	}

	/**
	 * Inequality operator
	 * @param rhs the right hand side item
	 * @return true if the items are unequal to each other.
	 */
	bool operator!=(const BuildItem rhs) const {
		return !operator==(rhs);
	}

};
struct TransitionGraph {
	std::string early,mid,late;	
};

/**
 * Manages build orders in three phases: early, mid, and late game.
 * Handles user command "transition"
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 * Added ability to research upgrades and tech.
 * @todo Add doxygen comments for old code
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
	void switchToPhase(const std::string& fileName);

	/** Returns the number of units of the specified type currently being produced.
	 * @deprecated This function should be in AgentManager instead */
	__declspec(deprecated) int countInProduction(const BWAPI::UnitType& type) const;

	/** Called each update to issue orders. */
	void computeActions();

	/** Notifies that an own unit has been destroyed. */
	void buildingDestroyed(const BWAPI::Unit* building);

	/** When a request to construct a new building is issued, no construction are
	 * allowed until the worker has moved to the build spot and started constructing
	 * the building. This is to avoid that the needed resources are not used up by
	 * other build orders. During this time the BuildPlanner is locked, and new 
	 * construction can only be done when unlock has been called. */
	void removeFromQueue(const BWAPI::UnitType& type);

	/** Removes a building from the build order. */
	void removeFirstOf(const BWAPI::UnitType& type);

	/**
	 * Removes the first of the specified item from the build order
	 * @param item the item to remove from the build order.
	 */
	void removeFirstOf(const BuildItem& item);

	/** Called when a worker that is constructing a building is destroyed. */
	void handleWorkerDestroyed(const BWAPI::UnitType& type, int workerID);

	/** Shows some debug info on screen. */
	void printGraphicDebugInfo() const;

	/** Is called when no build spot has been found for the specified type. Gives each build planner
	 * an opportunity to handle it. */
	void handleNoBuildspotFound(const BWAPI::UnitType& toBuild);

	/** Checks if more supply buildings are needed. */
	bool shallBuildSupply() const;

	/**
	 * Returns how many supplies that's currently being under construction
	 * @return number of supplies currently under construction
	 */
	int getSuppliesBeingBuiltCount() const;

	/**
	 * Checks if the next structure in the build order is of this type.
	 * @param type the type to check for. If type is not a supply provider and a supply
	 * provider is first it will check the second element in the build order instead.
	 * @return true if type is next in the build order
	 */
	bool nextIsOfType(const BWAPI::UnitType& type) const;

	/** Returns true if build order contains a unit of the specified type. */
	bool containsType(const BWAPI::UnitType& type) const;

	/** Adds a building to the build order queue. */
	void addBuilding(const BWAPI::UnitType& type);

	/**
	 * Adds an build item first, or second in the building queue. Supply depots are
	 * always added first and if a supply depot already is first the item will
	 * be added just after the supply depot.
	 * @param item the item to build or research.
	 */
	void addItemFirst(const BuildItem& item);

	/** Requests to expand the base. */
	void expand();

	/** Check if expansion is available. */
	bool isExpansionAvailable() const;

	/** Adds a refinery to the build order list. */
	void addRefinery();

	/** Checks if the specified BWAPI::TilePosition is covered by a detector buildings sight radius.
	 * @deprecated should not be here */
	__declspec(deprecated) static bool coveredByDetector(const BWAPI::TilePosition& pos);

	/** Morphs a Zerg drone to a building. */
	bool executeMorph(const BWAPI::UnitType& target, const BWAPI::UnitType& evolved);

	/** Returns true if the player is Terran.
	 * @deprecated should not be here */
	__declspec(deprecated) static bool isTerran();

	/** Returns true if the player is Protoss.
	 * @deprecated should not be here */
	__declspec(deprecated) static bool isProtoss();

	/** Returns true if the player is Zerg. 
	 * @deprecated should not be here */
	__declspec(deprecated) static bool isZerg();

	/**
	 * Finds a free worker to continue building on the specified building.
	 * Used when the assigned worker abandons its building or the worker was killed
	 * @param structure the structure to find another worker for
	 * @note only applicable for Terran
	 */
	void findAnotherBuilder(const BWAPI::Unit* structure);

private:
	BuildPlanner();
	/**
	 * @todo rework moveToQueue, a bit dangerous use at the moment when executing from
	 * executeOrder() as we don't know the index we just assume its 0. Better
	 * would be to either move an BuildItem and then search for it or to always for the
	 * first index to be moved.
	 */
	void moveToQueue(int buildOrderIndex, int unitId);
	bool executeOrder(const BuildItem& type);
	bool shallBuildSupplyDepot() const;
	std::string format(const BuildItem& type) const;
	bool hasResourcesLeft() const;
	int mineralsNearby(const BWAPI::TilePosition& center) const;
	bool canUpgrade(const BWAPI::UpgradeType& type, const BWAPI::Unit* unit) const;
	bool canResearch(const BWAPI::TechType& type, const BWAPI::Unit* unit) const;

	/**
	 * Checks if an upgrade or tech upgrade has been completed and was ordered
	 * to upgrade. If so, it removes it from the build queue.
	 */
	void checkUpgradeDone();


	std::vector<BuildItem> mBuildOrder;
	std::vector<BuildItem> mBuildQueue;
	int mLastCallFrame;
	std::vector<bats::CoreUnit> mCoreUnitsList;
	std::string mCurrentPhase;	

	ResourceManager* mResourceManager;
	CoverMap* mCoverMap;
	AgentManager* mAgentManager;

	static BuildPlanner* msInstance;
};
}