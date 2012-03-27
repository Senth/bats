#pragma once

#include "BTHAIModule/Source/UnitAgent.h"
#include "BTHAIModule/Source/SpottedObject.h"
#include "Squad.h"
#include <memory.h>

namespace bats {

/**
 * What a player's force is, includes both units and structures.
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 */
struct ForceData {
	int airAttackStr;
	int airDefendStr;
	int groundAttackStr;
	int groundDefendStr;

	int cRefineries;
	int cCommandCenters;
	int cFactories;
	int cAirports;
	int cBarracks;
	int cDefenseStructures;
	int cDetectorStructures;
	int cDetectorUnits;

	void reset()
	{
		airAttackStr = 0;
		airDefendStr = 0;
		groundAttackStr = 0;
		groundDefendStr = 0;

		cRefineries = 0;
		cCommandCenters = 0;
		cFactories = 0;
		cAirports = 0;
		cBarracks = 0;
		cDefenseStructures = 0;
		cDetectorStructures = 0;
		cDetectorUnits = 0;
	}

	void checkType(BWAPI::UnitType type)
	{
		if (type.isRefinery())
		{
			cRefineries++;
		}
		if (type.isResourceDepot())
		{
			cCommandCenters++;
		}
		if (type.isBuilding() && type.isDetector())
		{
			cDetectorStructures++;
		}
		if (!type.isBuilding() && type.isDetector())
		{
			cDetectorUnits++;
		}
		if (type.isBuilding() && type.canAttack())
		{
			cDefenseStructures++;
		}
		if (type.getID() == BWAPI::UnitTypes::Terran_Bunker.getID())
		{
			cDefenseStructures++;
		}
		if (type.getID() == BWAPI::UnitTypes::Terran_Starport.getID() || type.getID() == BWAPI::UnitTypes::Protoss_Stargate.getID())
		{
			cAirports++;	
		}
		if (type.getID() == BWAPI::UnitTypes::Terran_Barracks.getID() || type.getID() == BWAPI::UnitTypes::Protoss_Gateway.getID())
		{
			cBarracks++;
		}
		if (type.getID() == BWAPI::UnitTypes::Terran_Factory.getID() || type.getID() == BWAPI::UnitTypes::Protoss_Gateway.getID() || type.getID() == BWAPI::UnitTypes::Protoss_Robotics_Facility.getID())
		{
			cFactories++;
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

/** The ExplorationManager handles all tasks involving exploration of the game world.
 ** It issue orders to a number of units that is used as explorers, keep track of
 ** areas recently explored, and keep track of spotted resources or enemy buildings.
 *
 * The ExplorationManager is implemented as a singleton class.
 * Each class that needs to access ExplorationManager can request an instance,
 * and all classes shares the same ExplorationManager instance.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * 
 * Matteus Magnusson changed the code so that it works better with BATS. Updated the documentation
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class ExplorationManager {
public:
	/** Destructor */
	~ExplorationManager();

	/** Returns the instance of the class. Will create an instance if none exist.
	 * @return instance of the class. 
	 */
	static ExplorationManager* getInstance();

	/** Sets ExplorationManager to inactive. Is used when perfect information is activated. */
	void setInactive();

	/** Returns true if the ExplorationManager is active, false if not. It shall always
	 ** be active if no perfect information is available.
	 * @return true if the ExplorationManager is active, false if not. 
	 */
	bool isActive();

	/** Called each update to issue orders. */
	void computeActions();

	/** Returns the next position to explore for this squad. This will calculate
	 * a close valid position to go to explore. It will prioritize areas that haven't
	 * been explored for a while.
	 * @param squad the squad to return a position for
	 * @return the next position to squad for the specified squad
	 */
	BWAPI::TilePosition getNextToExplore(const std::tr1::shared_ptr<Squad>& squad);

	///** Searches for the next position to expand the base to. */
	//BWAPI::TilePosition searchExpansionSite();

	///** Returns the next position to expand the base to. */
	//BWAPI::TilePosition getExpansionSite();

	///** Sets the next position to expand the base to. */
	//void setExpansionSite(BWAPI::TilePosition pos);

	/** Shows all spotted objects as squares on the SC map. Use for debug purpose. */
	void printInfo();

	/** Notify the ExplorationManager that an enemy unit has been spotted.
	 * @param pUnit the unit that has been spotted.
	 */
	void addSpottedUnit(BWAPI::Unit* pUnit);

	/** Notify the ExplorationManager that an enemy unit has been destroyed. If the
	 * enemy unit was among the spotted units, it is removed from the list.
	 * @param pUnit the unit that has been destroyed.
	 */
	void unitDestroyed(BWAPI::Unit* pUnit);

	/** Returns the list of spotted enemy buildings.
	 * @return a list with all spotted units. 
	 */
	const std::vector<std::tr1::shared_ptr<SpottedObject>>& getSpottedBuildings() const;

	/** \codydoc getSpottedBuildings() */
	std::vector<std::tr1::shared_ptr<SpottedObject>>& getSpottedBuildings();

	/** Returns the closest enemy spotted building from a start position, or BWAPI::TilePosition(-1,-1) if 
	 * none was found. */
	BWAPI::TilePosition getClosestSpottedBuilding(BWAPI::TilePosition start);

	/** Calculates the number of spotted enemy structure within the specified range (in tiles).
	 * @param position the position to check if there are any enemy buildings in range.
	 * @param range the radius to check from the position
	 * @return number of enemy structures in range.
	 */
	int spottedBuildingsWithinRange(BWAPI::TilePosition position, int range);

	/**
	 * Returns true if any enemy structures have been spotted.
	 * @return true if any enemy structures have been spotted.
	 */
	bool buildingsSpotted();

	/** Shows some data about the enemy on screen. */
	void showIntellData();

	/** Returns true if a ground unit can reach position 'destination' from position 'source'.
	 * Uses BWTA.
	 * @param source the position a ground unit is located now.
	 * @param destination the location a ground unit want to move to.
	 * @return true if the ground unit can reach position 'destination' from 'source'.
	 * Else false
	 */
	static bool canReach(BWAPI::TilePosition source, BWAPI::TilePosition destination);

	/** Returns true if a unit can reach position 'destination'. 
	 * @param pUnit the unit to check if it can move
	 * @param destination the location pUnit wants to check
	 * @return true if the unit can reach position 'destination'. Else false
	 */
	static bool canReach(UnitAgent* pUnit, BWAPI::TilePosition destination);

	/** Sets that a region is explored.
	 * @pre The position must be the BWAPI::TilePosition for the center of the
	 * region.
	 * @param exploredPos the explored position
	 */
	void setExplored(const BWAPI::TilePosition& exploredPos);

	///**
	// * Scans for vulnerable enemy bases, i.e. bases without protection from detectors.
	// */
	//BWAPI::TilePosition scanForVulnerableBase();

	/** Checks if an enemy detector is covering the specified position.
	 * This can give a false negative, i.e. when an observer is located at that position
	 * and we don't have a detector in range.
	 * @param position the position we want to see if a detector is covering
	 * @return true if a visible enemy detector is covering the position.
	 */
	bool isEnemyDetectorCovering(BWAPI::TilePosition position);

	/** \copydoc isEnemyDetectorCovering() */
	bool isEnemyDetectorCovering(BWAPI::Position position);

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	ExplorationManager();

	std::vector<std::tr1::shared_ptr<SpottedObject>> mSpottedStructures;
	std::vector<std::tr1::shared_ptr<SpottedObject>> mSpottedUnits;

	std::vector<ExploreData> mExploreData;
	int getLastVisitFrame(BWTA::Region* region);

	ForceData mForceOwn;
	ForceData mForceEnemy;

	static ExplorationManager* mpsInstance;

	void calcEnemyForceData();
	void calcOwnForceData();

	void cleanup();

	bool mActive;
	int mFrameLastCall;

	//int mSiteSetFrame;
	//BWAPI::TilePosition mExpansionSite;

	/**
	 * When we shall calculate the specific force
	 */
	enum CalcTurns {
		CalcTurn_First,
		CalcTurn_Enemy = CalcTurn_First,
		CalcTurn_Our,
		CalcTurn_Player,
		CalcTurn_Lim
	};

	CalcTurns mCalcTurnCurrent;
};

}