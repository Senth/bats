#pragma once

#include "BTHAIModule/Source/UnitAgent.h"
#include "ExploreData.h"
#include <memory.h>

class SpottedObject;

namespace bats {

class Squad;

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
		else if (type.isResourceDepot())
		{
			cCommandCenters++;
		}
		else if (type.isBuilding() && type.isDetector())
		{
			cDetectorStructures++;
		}
		else if (!type.isBuilding() && type.isDetector())
		{
			cDetectorUnits++;
		}
		else if (type.isBuilding() && type.canAttack())
		{
			cDefenseStructures++;
		}
		else if (type == BWAPI::UnitTypes::Terran_Bunker ||
			type == BWAPI::UnitTypes::Terran_Missile_Turret ||
			type == BWAPI::UnitTypes::Protoss_Photon_Cannon ||
			type == BWAPI::UnitTypes::Zerg_Sunken_Colony ||
			type == BWAPI::UnitTypes::Zerg_Spore_Colony)
		{
			cDefenseStructures++;
		}
		else if (type == BWAPI::UnitTypes::Terran_Starport ||
			type == BWAPI::UnitTypes::Protoss_Stargate)
		{
			cAirports++;	
		}
		else if (type == BWAPI::UnitTypes::Terran_Barracks ||
			type == BWAPI::UnitTypes::Protoss_Gateway)
		{
			cBarracks++;
		}
		else if (type == BWAPI::UnitTypes::Terran_Factory || 
			type == BWAPI::UnitTypes::Protoss_Robotics_Facility)
		{
			cFactories++;
		}
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
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 * Changed the code so that it works better with BATS.
 * Updated the documentation, added, and removed functions.
 */
class ExplorationManager {
public:
	/**
	 * Destructor
	 */
	~ExplorationManager();

	/**
	 * Returns the instance of the class. Will create an instance if none exist.
	 * @return instance of the class. 
	 */
	static ExplorationManager* getInstance();

	/** 
	 * Called each update to issue orders.
	 */
	void update();

	/**
	 * Returns the next position to explore for this squad. This will calculate
	 * a close valid position to go to explore. It will prioritize areas that haven't
	 * been explored for a while.
	 * @param squad the squad to return a position for
	 * @return the next position to squad for the specified squad
	 */
	BWAPI::TilePosition getNextToExplore(const std::tr1::shared_ptr<Squad>& squad);

	/**
	 * Shows all spotted objects as squares on the SC map. Use for debug purpose.
	 */
	void printGraphicDebugInfo() const;

	/**
	 * Notify the ExplorationManager that an enemy unit has been spotted.
	 * @param unit the unit that has been spotted.
	 */
	void addSpottedUnit(const BWAPI::Unit* unit);

	/**
	 * Notify the ExplorationManager that an enemy unit has been destroyed. If the
	 * enemy unit was among the spotted units, it is removed from the list.
	 * @param unit the unit that has been destroyed.
	 */
	void unitDestroyed(const BWAPI::Unit* unit);

	/** Returns the list of spotted enemy buildings.
	 * @return a list with all spotted units. 
	 */
	const std::vector<std::tr1::shared_ptr<SpottedObject>>& getSpottedBuildings() const;

	/**
	 * \copydoc getSpottedBuildings() const
	 */
	std::vector<std::tr1::shared_ptr<SpottedObject>>& getSpottedBuildings();

	/** 
	 * Returns the closest enemy spotted building from a start position.
	 * @param startPosition the position we want to start searching from
	 * @return position of the closest building that has been spotted (first), including the
	 * squared distance (second).
	 * If no building was found it will return BWAPI::TilePositions::Invalid (first).
	 */
	std::pair<BWAPI::TilePosition, int> getClosestSpottedBuilding(const BWAPI::TilePosition& startPosition) const;

	/** 
	 * Calculates the number of spotted enemy structure within the specified range (in tiles).
	 * @param position the position to check if there are any enemy structures in range.
	 * @param range the radius to check from the position
	 * @return number of enemy structures in range.
	 */
	int countSpottedBuildingsWithinRange(const BWAPI::TilePosition& position, int range) const;

	/**
	 * Calculates whether there exist a spotted enemy structure within the specified range
	 * from the specified position.
	 * @param position the position to check if there are any enemy structures in range.
	 * @param range the radius to check from the position.
	 * @return true if an enemy structure exists within the air range of the position.
	 * @todo Implement functionality use ground range and not always air range.
	 */
	bool hasSpottedBuildingWithinRange(const BWAPI::TilePosition& position, int range) const;

	/**
	 * Returns true if any enemy structures have been spotted.
	 * @return true if any enemy structures have been spotted.
	 */
	bool hasSpottedBuilding() const;

	/** 
	 * Shows some data about the enemy on screen.
	 */
	void showIntellData() const;

	/**
	 * Returns true if a ground unit can reach position 'destination' from position 'source'.
	 * Uses BWTA.
	 * @param source the position a ground unit is located now.
	 * @param destination the location a ground unit want to move to.
	 * @return true if the ground unit can reach position 'destination' from 'source'.
	 * Else false
	 * @deprecated Will be removed in a later version, this function does not belong here.
	 */
	static bool canReach(const BWAPI::TilePosition& source, const BWAPI::TilePosition& destination);

	/**
	 * Returns true if a unit can reach position 'destination'. 
	 * @param unit the unit to check if it can move
	 * @param destination the location unit wants to check
	 * @return true if the unit can reach position 'destination', else false
	 * @deprecated Will be removed in a later version, this function does not belong here.
	 */
	static bool canReach(const BaseAgent* unit, const BWAPI::TilePosition& destination);

	/** 
	 * Checks if an enemy detector is covering the specified position.
	 * This can give a false negative, i.e. when an observer is located at that position
	 * and we don't have a detector in range.
	 * @param position the position we want to see if a detector is covering
	 * @return true if a visible enemy detector is covering the position.
	 * @deprecated Will be removed in a later version, this function does not belong here?
	 */
	bool isEnemyDetectorCovering(const BWAPI::TilePosition& position) const;

	/**
	 * \copydoc isEnemyDetectorCovering()
	 */
	bool isEnemyDetectorCovering(const BWAPI::Position& position) const;

	/**
	 * Find expansions that haven't been scouted for X seconds. X is specified by
	 * config::attack_coordinator::EXPANSION_NOT_CHECKED_TIME and can be overridden in
	 * the config file. This function can return occupied bases.
	 * If you want to only get empty expansions. Use removeOccupiedExpansions() and use
	 * the return vector from this function as an argument to removeOccupiedExpansions().
	 * @return a vector with all expansions that haven't been scouted
	 * for the specified amount of time.
	 */
	std::vector<BWAPI::TilePosition> findNotCheckedExpansions() const;

	/**
	 * Removes expansions locations that are occupied.
	 * @param[in,out] expansionPositions a list with all expansion positions. Will remove all
	 * occupied expansions from this parameter.
	 */
	void removeOccupiedExpansions(std::vector<BWAPI::TilePosition>& expansionPositions) const;

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	ExplorationManager();
	
	/**
	 * Calculates the enemy force strength
	 */
	void calcEnemyForceData();
	
	/**
	 * Calculates our force strength
	 */
	void calcOwnForceData();
	
	/**
	 * Remove buildings that have been moved or destroyed
	 */
	void cleanup();

	std::vector<std::tr1::shared_ptr<SpottedObject>> mSpottedStructures;
	std::vector<std::tr1::shared_ptr<SpottedObject>> mSpottedUnits;

	std::vector<ExploreData> mExploreData;

	ForceData mForceOwn;
	ForceData mForceEnemy;

	int mFrameLastCall;

	static ExplorationManager* msInstance;

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