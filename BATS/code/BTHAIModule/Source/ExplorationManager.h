#ifndef __EXPLORATIONMANAGER_H__
#define __EXPLORATIONMANAGER_H__

#include "UnitAgent.h"
#include "SpottedObject.h"
#include "Squad.h"

struct ForceData {
	int airAttackStr;
	int airDefendStr;
	int groundAttackStr;
	int groundDefendStr;

	int noRefineries;
	int noCommandCenters;
	int noFactories;
	int noAirports;
	int noBarracks;
	int noDefenseStructures;
	int noDetectorStructures;
	int noDetectorUnits;

	void reset()
	{
		airAttackStr = 0;
		airDefendStr = 0;
		groundAttackStr = 0;
		groundDefendStr = 0;

		noRefineries = 0;
		noCommandCenters = 0;
		noFactories = 0;
		noAirports = 0;
		noBarracks = 0;
		noDefenseStructures = 0;
		noDetectorStructures = 0;
		noDetectorUnits = 0;
	}

	void checkType(BWAPI::UnitType type)
	{
		if (type.isRefinery())
		{
			noRefineries++;
		}
		if (type.isResourceDepot())
		{
			noCommandCenters++;
		}
		if (type.isBuilding() && type.isDetector())
		{
			noDetectorStructures++;
		}
		if (!type.isBuilding() && type.isDetector())
		{
			noDetectorUnits++;
		}
		if (type.isBuilding() && type.canAttack())
		{
			noDefenseStructures++;
		}
		if (type.getID() == BWAPI::UnitTypes::Terran_Bunker.getID())
		{
			noDefenseStructures++;
		}
		if (type.getID() == BWAPI::UnitTypes::Terran_Starport.getID() || type.getID() == BWAPI::UnitTypes::Protoss_Stargate.getID())
		{
			noAirports++;	
		}
		if (type.getID() == BWAPI::UnitTypes::Terran_Barracks.getID() || type.getID() == BWAPI::UnitTypes::Protoss_Gateway.getID())
		{
			noBarracks++;
		}
		if (type.getID() == BWAPI::UnitTypes::Terran_Factory.getID() || type.getID() == BWAPI::UnitTypes::Protoss_Gateway.getID() || type.getID() == BWAPI::UnitTypes::Protoss_Robotics_Facility.getID())
		{
			noFactories++;
		}
	}
};

struct ExploreData {
	BWAPI::TilePosition center;
	int lastVisitFrame;
	
	ExploreData(BWAPI::Position tCenter)
	{
		center = BWAPI::TilePosition(tCenter);
		lastVisitFrame = 0;
	}

	int getX()
	{
		return center.x();
	}

	int getY()
	{
		return center.y();
	}

	bool matches(BWTA::Region* region)
	{
		if (region == NULL)
		{
			return false;
		}
		BWAPI::TilePosition tCenter = BWAPI::TilePosition(region->getCenter());
		return matches(tCenter);
	}

	bool matches(BWAPI::TilePosition tCenter)
	{
		double dist = tCenter.getDistance(center);
		if (dist <= 2)
		{
			return true;
		}
		return false;
	}
};

/** The ExplorationManager handles all tasks involving exploration of the game world. It issue orders to a number of units
 * that is used as explorers, keep track of areas recently explored, and keep track of spotted resources or enemy buildings.
 *
 * The ExplorationManager is implemented as a singleton class. Each class that needs to access ExplorationManager can request an instance,
 * and all classes shares the same ExplorationManager instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ExplorationManager {

private:
	std::vector<SpottedObject*> spottedBuildings;
	std::vector<SpottedObject*> spottedUnits;
	
	std::vector<ExploreData> exploreData;
	int getLastVisitFrame(BWTA::Region* region);
	
	ForceData ownForce;
	ForceData enemyForce;

	ExplorationManager();
	static ExplorationManager* instance;
	static bool instanceFlag;
	
	void calcEnemyForceData();
	void calcOwnForceData();

	void cleanup();

	bool active;

	int lastCallFrame;

	int siteSetFrame;
	BWAPI::TilePosition expansionSite;

public:
	/** Destructor */
	~ExplorationManager();

	/** Returns the instance of the class. */
	static ExplorationManager* getInstance();

	/** Sets ExplorationManager to inactive. Is used when perfect information is activated. */
	void setInactive();

	/** Returns true if the ExplorationManager is active, false if not. */
	bool isActive();

	/** Called each update to issue orders. */
	void computeActions();

	/** Returns the next position to explore for this squad. */
	BWAPI::TilePosition getNextToExplore(Squad* squad);

	/** Searches for the next position to expand the base to. */
	BWAPI::TilePosition searchExpansionSite();

	/** Returns the next position to expand the base to. */
	BWAPI::TilePosition getExpansionSite();

	/** Sets the next position to expand the base to. */
	void setExpansionSite(BWAPI::TilePosition pos);

	/** Shows all spotted objects as squares on the SC map. Use for debug purpose. */
	void printInfo();

	/** Notifies about an enemy unit that has been spotted. */
	void addSpottedUnit(BWAPI::Unit* unit);

	/** Notifies that an enemy unit has been destroyed. If the destroyed unit was among
	 * the spotted units, it is removed from the list. */
	void unitDestroyed(BWAPI::Unit* unit);

	/** Returns the list of spotted enemy buildings. */
	std::vector<SpottedObject*> getSpottedBuildings();

	/** Returns the closest enemy spotted building from a start position, or BWAPI::TilePosition(-1,-1) if 
	 * none was found. */
	BWAPI::TilePosition getClosestSpottedBuilding(BWAPI::TilePosition start);

	/** Calculates the number of spotted enemy buildings within the specified range (in tiles). */
	int spottedBuildingsWithinRange(BWAPI::TilePosition pos, int range);

	/** Returns true if any enemy buildings have been spotted. */
	bool buildingsSpotted();

	/** Shows some data about the enemy on screen. */
	void showIntellData();

	/** Returns true if a ground unit can reach position b from position a.
	 * Uses BWTA. */
	static bool canReach(BWAPI::TilePosition a, BWAPI::TilePosition b);

	/** Returns true if an agent can reach position b. */
	static bool canReach(BaseAgent* agent, BWAPI::TilePosition b);

	/** Sets that a region is explored. The position must be the BWAPI::TilePosition for the center of the
	 * region. */
	void setExplored(BWAPI::TilePosition goal);

	/** Scans for vulnerable enemy bases, i.e. bases without protection from detectors. */
	BWAPI::TilePosition scanForVulnerableBase();

	/** Checks if an enemy detector is covering the specified position. */
	bool isDetectorCovering(BWAPI::TilePosition pos);

	/** Checks if an enemy detector is covering the specified position. */
	bool isDetectorCovering(BWAPI::Position pos);

	/** Returns true if an enemy is Protoss. */
	static bool enemyIsProtoss();

	/** Returns true if an enemy is Zerg. */
	static bool enemyIsZerg();

	/** Returns true if an enemy is Terran. */
	static bool enemyIsTerran();

	/** All enemy races are currently unknown. */
	static bool enemyIsUnknown();
};

#endif