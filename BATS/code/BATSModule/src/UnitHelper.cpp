#include "UnitHelper.h"
#include "Helper.h"

#include <BWAPI/Position.h>
#include <BWAPI/Constants.h>

using namespace bats;
using namespace BWAPI;
using namespace std;

bool bats::UnitHelper::isGasStructure(const BWAPI::Unit* unit) {
	if (unit != NULL) {
		const UnitType& unitType = unit->getType();

		if (unitType == UnitTypes::Resource_Vespene_Geyser ||
			unitType == UnitTypes::Terran_Refinery ||
			unitType == UnitTypes::Protoss_Assimilator ||
			unitType == UnitTypes::Zerg_Extractor)
		{
			return true;
		}
	}

	return false;
}

bool bats::UnitHelper::isEnemyWithinRadius(const BWAPI::TilePosition& center, int radius) {
	const std::set<Unit*>& units = Broodwar->getUnitsInRadius(Position(center), radius * TILE_SIZE);
	std::set<Unit*>::const_iterator unitIt;
	for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
		if ((*unitIt)->getPlayer()->isEnemy(Broodwar->self())) {
			return true;
		}
	}

	return false;
}

vector<Unit*> bats::UnitHelper::getEnemyUnits() {
	vector<Unit*> enemyUnits;

	const set<Player*>& players = Broodwar->enemies();
	set<Player*>::const_iterator playerIt;
	for (playerIt = players.begin(); playerIt != players.end(); ++playerIt) {
		const set<Unit*>& units = (*playerIt)->getUnits();
		enemyUnits.insert(enemyUnits.end(), units.begin(), units.end());
	}

	return enemyUnits;
}

vector<Unit*> bats::UnitHelper::getTeamUnits(bool includeAllies, bool includeSelf) {
	set<Player*> players;
	if (includeAllies) {
		players = Broodwar->allies();
	}
	if (includeSelf) {
		players.insert(Broodwar->self());
	}

	vector<Unit*> teamUnits;

	set<Player*>::const_iterator playerIt;
	for (playerIt = players.begin(); playerIt != players.end(); ++playerIt) {
		const set<Unit*>& units = (*playerIt)->getUnits();
		teamUnits.insert(teamUnits.end(), units.begin(), units.end());
	}

	return teamUnits;
}

BWAPI::TilePosition bats::UnitHelper::findEnemyPositionWithinRadius(const BWAPI::TilePosition& center, int radius) {
	const std::set<Unit*>& units = Broodwar->getUnitsInRadius(Position(center), radius * TILE_SIZE);
	std::set<Unit*>::const_iterator unitIt;
	for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
		if ((*unitIt)->getPlayer()->isEnemy(Broodwar->self()) && (*unitIt)->getTilePosition().isValid()) {
			return (*unitIt)->getTilePosition();
		}
	}

	return TilePositions::Invalid;
}

std::pair<Unit*,int> bats::UnitHelper::getClosestAlliedStructure(const BWAPI::TilePosition& position) {
	Unit* pClosestUnit = NULL;
	int closestDistance = INT_MAX;

	// Get all our structures
	const std::set<Unit*>& units = Broodwar->self()->getUnits();
	std::set<Unit*>::const_iterator unitIt;
	for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
		if ((*unitIt)->getType().isBuilding()) {
			int diffDistance = getSquaredDistance((*unitIt)->getTilePosition(), position);
			if (diffDistance < closestDistance) {
				closestDistance = diffDistance;
				pClosestUnit = (*unitIt);
			}
		}
	}

	// Get all allied structures
	const std::set<Player*>& allies = Broodwar->allies();
	std::set<Player*>::const_iterator alliedIt;
	for (alliedIt = allies.begin(); alliedIt != allies.end(); ++alliedIt) {
		const std::set<Unit*>& units = (*alliedIt)->getUnits();
		for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
			if ((*unitIt)->getType().isBuilding()) {
				int diffDistance = getSquaredDistance((*unitIt)->getTilePosition(), position);
				if (diffDistance < closestDistance) {
					closestDistance = diffDistance;
					pClosestUnit = (*unitIt);
				}
			}
		}
	}

	return std::make_pair(pClosestUnit, closestDistance);
}

bool bats::UnitHelper::unitsInArea(const vector<BWAPI::Unit*>& units, const TilePosition& posTileMin, const TilePosition& posTileMax, int excludeId) {
	Position posMin(posTileMin);
	Position posMax(posTileMax + TilePosition(1,1));

	for (size_t i = 0; i < units.size(); ++i) {
		if (units[i]->getID() != excludeId) {

			// It's enough if the right side is inside the left side of the box
			// and bottom unit side is below upper box side
			Position unitPosMin(units[i]->getRight(), units[i]->getBottom());
			Position unitPosMax(units[i]->getLeft(), units[i]->getTop());

			if (unitPosMin.x() >= posMin.x() && unitPosMax.x() <= posMax.x() &&
				unitPosMin.y() >= posMin.y() && unitPosMax.y() <= posMax.y())
			{
				return true;
			}
		}
	}

	return false;
}