#pragma once

#include <BWAPI/Unit.h>
#include <BWAPI/Player.h>
#include <BWAPI/Game.h>
#include <BWAPI/TilePosition.h>
#include <vector>

namespace bats {
namespace UnitHelper {

/**
 * Checks if a unit is a gas building. This includes Resource_Vespene_Geyser, Terran_Refinery,
 * Protoss_Assimilator, Zerg_Extractor
 * @return true if the unit is a gas building.
 */
bool isGasStructure(const BWAPI::Unit* unit);

/**
 * A function to check if a unit belongs to us
 * @param unit unit to check if it belongs to us
 * @return true if the unit belongs to us, else false
 */
inline bool isOurs(const BWAPI::Unit* unit) {
	return unit->getPlayer() == BWAPI::Broodwar->self();
}

/**
 * Check if a unit belongs to an ally
 * @param unit unit to check if it belongs to an ally
 * @return true if the unit belongs to an ally.
 */
inline bool isAllied(const BWAPI::Unit* unit) {
	return BWAPI::Broodwar->self()->isAlly(unit->getPlayer()) &&
		BWAPI::Broodwar->self() != unit->getPlayer();
}

/**
 * Checks if a unit belongs to an enemy
 * @param unit unit to check if it belongs to an enemy
 * @return true if the unit belongs to an enemy.
 */
inline bool isEnemy(const BWAPI::Unit* unit) {
	return BWAPI::Broodwar->self()->isEnemy(unit->getPlayer());
}

/**
 * Checks if a unit is neutral
 * @param unit unit to check if it is neutral
 * @return true if the unit is neutral
 */
inline bool isNeutral(const BWAPI::Unit* unit) {
	return unit->getPlayer()->isNeutral();
}

/**
 * Returns true if an enemy is within the specified radius from the position.
 * @param center the position to check from.
 * @param radius maximum range from the position the enemy can be, in tile size.
 * @return true if an enemy is found within the radius.
 */
bool isEnemyWithinRadius(const BWAPI::TilePosition& center, int radius);

/**
 * Returns all enemy units (from all enemies)
 * @return vector with all enemy units
 */
std::vector<BWAPI::Unit*> getEnemyUnits();

/**
 * Returns all team units, both from all allies and our units (if specified)
 * @param includeAllies includes units from allied players, defaults to true
 * @param includeSelf includes units from the bot itself, defaults to true
 * @return vector with all the team's units.
 */
std::vector<BWAPI::Unit*> getTeamUnits(bool includeAllies = true, bool includeSelf = true);

/**
 * Searches for an enemy position within the specified radius. This version returns
 * the first available position and the enemies are not sorted in any way.
 * @param center the position to check from.
 * @param radius maximum range from the center the enemy shall be, in tile size.
 * @return position of an enemy within the radius, TilePositions::Invalid if none was found.
 */
BWAPI::TilePosition findEnemyPositionWithinRadius(const BWAPI::TilePosition& center, int radius);

/**
* Returns the closest allied structure (including our structures).
* @param[in] position where we measure from
* @return closest allied structure including the <strong>squared</strong> distance.
*/
std::pair<BWAPI::Unit*,int> getClosestAlliedStructure(const BWAPI::TilePosition& position);

/**
 * Checks if there are any units in the specified area between minPos and maxPos. An optional
 * excludeId can be used, it will not check this unit.
 * @param units units to check if they are in the specified area
 * @param posTileMin the minimum position of the area, i.e. top left
 * @param posTileMax the maximum position of the area, i.e. bottom right
 * @param excludeId excludes this id from the check, useful when a builder is searching for
 * a spot to build. Defaults to -1 (invalid)
 * @return true if units where found in the area, false otherwise
 */
bool unitsInArea(
	const std::vector<BWAPI::Unit*>& units,
	const BWAPI::TilePosition& posTileMin,
	const BWAPI::TilePosition& posTileMax,
	int excludeId = -1
);
}
}