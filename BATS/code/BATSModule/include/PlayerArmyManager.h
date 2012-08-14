#pragma once

#include "SquadDefs.h"
#include "Config.h"
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <BWAPI/Unit.h>
#include <BWAPI/Position.h>

// Namespace for the project
namespace bats {

/**
 * Manages allies virtual squads. It created, splits, and merges units that are within the squads
 * Variables include_distance and exclude_distance under [classification.squad]
 * @todo split squads by players, as of now the players units are grouped into the same squad
 * if they are close to each other even if they belong to two different allies.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class PlayerArmyManager : public config::OnConstantChangedListener {
public:
	/**
	 * Destructor
	 */
	virtual ~PlayerArmyManager();

	/**
	 * Returns the instance of AlliedArmyManager.
	 * @return instance of AlliedArmyManager.
	 */
	static PlayerArmyManager* getInstance();

	/**
	 * Updates the squads. Splits squads, moves units from one squad to another, and disbands
	 * squads if no units is within the squad.
	 */
	void update();

	/**
	 * Called when a constant has been updated (that this class listens to).
	 * Currently it only listens these variables:
	 * \code
	 * [classification.squad]
	 * grid_square_distance
	 * \endcode
	 */
	void onConstantChanged(config::ConstantName constantName);

	/**
	 * Add an allied unit to the manager. Do this when a unit is created.
	 * @param unit the new unit 
	 */
	void addUnit(BWAPI::Unit* unit);

	/**
	 * Remove an allied unit from the manager. Do this when a unit is killed.
	 * @param unit the killed unit.
	 */
	void removeUnit(BWAPI::Unit* unit);

	/**
	 * Prints graphical debug information depending on the current
	 * GRAPHICS_VERBOSITY.
	 */
	void printGraphicDebugInfo() const;

	/**
	 * Returns the biggest squad if one exists.
	 * @return biggest squad, if no squad exist it will return NULL instead
	 */
	AlliedSquadCstPtr getAlliedFrontalSquad() const;

	/**
	 * Returns all allied squads of the specified type
	 * @tparam SquadType the type of squads to return. Probably AlliedSquad or EnemySquad
	 * @return all squads of the specified type.
	 */
	template<typename SquadType>
	std::vector<std::tr1::shared_ptr<const SquadType>> getSquads() const;

	/**
	 * Finds the closest squad to the specified position. An optional distance parameter
	 * can be set not to return any squad further away than this distance.
	 * @param position from where to find the closest squad
	 * @param distanceMax maximum distance away the squad can be, defaults to INT_MAX
	 * @return a pair where first is the squad (NULL if no squad was found) and second is the
	 * squared distance (INT_MAX if no squad was found).
	 */
	template <typename SquadType>
	std::pair<std::tr1::shared_ptr<const SquadType>, int> getClosestSquad(
		const BWAPI::TilePosition& position,
		int distanceMax = INT_MAX
	) const;

	/**
	 * Returns a vector with all squads within the specified distance from the position.
	 * @param position from where to search for the squads
	 * @param distanceMax maximum distance away the squad can be
	 * @param bSort if we want to sort the vector, shorter distance will be first in the
	 * vector, defaults to false.
	 * @return vector with a pair where first is the squad and second is the squared distance
	 * to the squad. If no squads are found within the distance an empty vector is returned.
	 */
	template <typename SquadType>
	std::vector<std::pair<std::tr1::shared_ptr<const SquadType>, int>> getSquadsWithin(
		const BWAPI::TilePosition& position,
		int distanceMax,
		bool bSort = false
	) const;

private:
	/**
	 * Singleton constructor to enforce singleton usage.
	 */
	PlayerArmyManager();

	/**
	 * Rearranges the squads so that all limitations are met. Meaning it will split squads,
	 * move units from one squad to another squad if squads are within include_distance or
	 * outside exclude_distance. This has to be called twice to update both allied squads
	 * and enemy squads.
	 * @param updateAllied set to true to update allied squads, false to update enemy squads.
	 */
	void rearrangeSquads(bool updateAllied);

	/**
	 * Checks if the two units are within exclude_distance
	 * @param unitA one of the units
	 * @param unitB the other unit
	 * @return true if distance between the two units are less or equal to exclude_distance
	 */
	bool withinExcludeDistance(BWAPI::Unit* unitA, BWAPI::Unit* unitB) const;

	/**
	 * Checks if the two units are within include_distance
	 * @param unitA one of the units
	 * @param unitB the other unit
	 * @return true if distance between the two units are less or equal to include_distance
	 */
	bool withinIncludeDistance(BWAPI::Unit* unitA, BWAPI::Unit* unitB) const;

	/**
	 * Returns the grid position of the unit. If the unit is inside a transportation it will
	 * use the transportation's position instead of the unit's.
	 * @param unit unit that check the corresponding grid placement of
	 * @pre unit is not NULL
	 * @return grid position of the unit.
	 */
	BWAPI::Position getGridPosition(BWAPI::Unit* unit) const;

	/**
	 * Returns true if the position is the same or a border grid position, not diagonally border.
	 * Used for when checking if units is withing exclude_distance with 100% certainty.
	 * @param centerPosition the origin of the unit to check
	 * @param checkPosition the position to check if it's a border or same position as centerPosition.
	 * @return true if checkPosition is same or border position as centerPosition (in grid units).
	 */
	bool isSameOrBorderGridPosition(const BWAPI::Position& centerPosition, const BWAPI::Position& checkPosition) const;

	/**
	 * Returns a valid range to iterate through the grid position, with a distance away from
	 * the center position.
	 * @note that the maximum range still is in bounds, so you have to use less than or equal to,
	 * x <= second.x(), not just less than x < second.x().
	 * @param centerPosition the center position
	 * @param range how many coordinates away it shall at max use.
	 * @pre centerPosition is a valid position
	 * @return a valid interval of coordinates, where first is the minimum coordinates and second
	 * is the maximum coordinates.
	 * @section Example
	 * If 2D array's size is 9x10 (max index x=8, y=9). And you enter center position
	 * 7 and range with 2. It would have a range of 5–9 (both x and y), but x=9 is an invalid
	 * index and will therefor return the range (5–8, 5–9).
	 */
	std::pair<BWAPI::Position, BWAPI::Position> getValidGridRange(const BWAPI::Position centerPosition, int range) const;

	/**
	 * Recursively add units, that are close to this unit, to the specified squad.
	 * @param unit position to check close-by units.
	 * @param squadId the squad of the original unit, i.e. the squad that units shall be added to
	 * if the unit already belongs to this squad, nothing is changed, if the squad does not
	 * belong to this squad, it is removed from the old squad and added to this.
	 */
	void addCloseUnitsToSquad(BWAPI::Unit* unit, PlayerSquadId squadId);

	/**
	 * Helper function, sets the unit as checked. Both erases it from mUnitsToCheck and sets
	 * it as checked in mGridUnits.
	 * @param unit the unit to be set as checked.
	 */
	void setUnitAsChecked(BWAPI::Unit* unit);

	/**
	 * Helper function, sets the unit as checked. Both erases it from mUnitsToCheck and sets
	 * it as checked in mGridUnits.
	 * @param unitIt the unit to be set as checked.
	 * @return iterator returned by mUnitsToBeChecked.erase(unitIt);
	 */
	std::map<BWAPI::Unit*, PlayerSquadId>::const_iterator setUnitAsChecked(const std::map<BWAPI::Unit*, PlayerSquadId>::const_iterator& unitIt);

	/**
	 * Recalculate the lookup table. Shall be called whenever grid_square_distance is
	 * changed.
	 */
	void recalculateLookupTable();

	/**
	 * Disbands all empty squads
	 */
	void disbandEmptySquads();

	/**
	 * Updates the current allied frontal squad, to actually be the biggest one.
	 */
	void setAlliedFrontalSquad();

	/**
	 * Add a newly created AlliedSquad to the squad list.
	 * @param pSquad pointer to the newly created squad
	 */
	void addSquad(PlayerSquadPtr pSquad);

	/**
	 * Removes an AlliedSquad from the squad list and deletes it
	 * @param squadId the squad id of the squad
	 */
	void removeSquad(PlayerSquadId squadId);

	/**
	 * Adds a unit to the grid with the correct position. I.e. if it's in a transportation
	 * it will set the unit's position to where the transportation is.
	 * @param unit the unit to add to the grid
	 */
	void addUnitToGrid(BWAPI::Unit* unit);

	int mLastFrameUpdate;
	std::vector<PlayerSquadPtr> mSquads;
	std::map<BWAPI::Unit*, PlayerSquadId> mUnitSquad; /**< A unit bound to a squad id */

	/** Look-up table for where the unit is located. */
	std::vector<std::vector<BWAPI::Position>> mLookupTableGridPosition;

	// Only temporary variables, placed here to avoid sending them as parameters
	// over various functions
	std::vector<std::vector<std::map<BWAPI::Unit*, bool>>> mGridUnits;
	std::map<BWAPI::Unit*, PlayerSquadId> mUnitsToCheck;
	

	static PlayerArmyManager* mpsInstance;
};


//////////////////////////////////////////////////////////////////////////
//////					TEMPLATE IMPLEMENTATION						//////
//////////////////////////////////////////////////////////////////////////
template<typename SquadType>
std::vector<std::tr1::shared_ptr<const SquadType>> PlayerArmyManager::getSquads() const {
	std::vector<std::tr1::shared_ptr<const SquadType>> squads;

	for (size_t i = 0; i < mSquads.size(); ++i) {
		std::tr1::shared_ptr<const SquadType> pSquad = std::tr1::dynamic_pointer_cast<const SquadType>(mSquads[i]);
		if (NULL != pSquad) {
			squads.push_back(pSquad);
		}
	}

	return squads;
}

template<typename SquadType>
std::pair<std::tr1::shared_ptr<const SquadType>, int> PlayerArmyManager::getClosestSquad(
	const BWAPI::TilePosition& position,
	int distanceMax) const
{
	typedef std::tr1::shared_ptr<const SquadType> SquadTypeCstPtr;
	std::pair<SquadTypeCstPtr, int> closestSquad;
	if (distanceMax == INT_MAX) {
		closestSquad.second = distanceMax;
	} else {
		closestSquad.second = distanceMax * distanceMax;
	}

	for (size_t i = 0; i < mSquads.size(); ++i) {
		SquadTypeCstPtr pSquad = std::tr1::dynamic_pointer_cast<const SquadType>(mSquads[i]);
		if (NULL != pSquad && pSquad->getCenter() != TilePositions::Invalid) {
			int distanceSquared = getSquaredDistance(position, pSquad->getCenter());

			if (distanceSquared < closestSquad.second) {
				closestSquad.second = distanceSquared;
				closestSquad.first = pSquad;
			}
		}
	}

	return closestSquad;
}

template<typename SquadType>
std::vector<std::pair<std::tr1::shared_ptr<const SquadType>, int>> PlayerArmyManager::getSquadsWithin(
	const BWAPI::TilePosition& position,
	int distanceMax,
	bool bSort) const
{
	typedef std::tr1::shared_ptr<const SquadType> SquadTypeCstPtr;
	/// @todo add parameter to check if returning allied or enemy squad
	int distanceMaxSquared = distanceMax * distanceMax;
	std::vector<std::pair<SquadTypeCstPtr, int>> foundSquads;

	for (size_t i = 0; i < mSquads.size(); ++i) {
		SquadTypeCstPtr pSquad = std::tr1::dynamic_pointer_cast<const SquadType>(mSquads[i]);
		if (NULL != pSquad && pSquad->getCenter() != TilePositions::Invalid) {
			int distanceSquared = getSquaredDistance(position, pSquad->getCenter());

			if (distanceSquared <= distanceMaxSquared) {
				foundSquads.push_back(make_pair(pSquad, distanceSquared));
			}
		}
	}

	if (bSort) {
		std::sort(foundSquads.begin(), foundSquads.end(), PairCompareSecond<SquadTypeCstPtr, int>());
	}

	return foundSquads;
}
}