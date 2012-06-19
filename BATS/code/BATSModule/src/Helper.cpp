#include "Helper.h"
#include <ostream>
#include <BWAPI/Game.h>
#include <set>
#include <cfloat>

using namespace BWAPI;
using namespace bats;

std::pair<Unit*,int> bats::getClosestAlliedStructure(const TilePosition& position) {
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

TilePosition bats::getClosestBorder(const TilePosition& position) {
	// Which horizontal border is closest, left or right?
	int leftDiff = position.x();
	int rightDiff = Broodwar->mapWidth() -1 - position.x();
	TilePosition closestBorderX = position;
	int horizontalDiff;
	if (leftDiff < rightDiff) {
		closestBorderX.x() = 0;
		horizontalDiff = leftDiff;
	} else {
		closestBorderX.x() = Broodwar->mapWidth() - 1;
		horizontalDiff = rightDiff;
	}

	// Which vertical border is closest, left or right?
	int topDiff = position.y();
	int bottomDiff = Broodwar->mapHeight() -1 - position.y();
	TilePosition closestBorderY = position;
	int verticalDiff;
	if (topDiff < bottomDiff) {
		closestBorderY.y() = 0;
		verticalDiff = topDiff;
	} else {
		closestBorderY.y() = Broodwar->mapHeight() - 1;
		verticalDiff = bottomDiff;
	}

	// Is the position closer to the vertical or horizontal border?
	if (horizontalDiff < verticalDiff) {
		return closestBorderX;
	} else {
		return closestBorderY;
	}
}

bats::Borders bats::getAtWhichBorder(const TilePosition& borderPosition) {
	if (borderPosition.x() == 0) {
		return Border_Left;
	} else if (borderPosition.y() == 0) {
		return Border_Top;
	} else if (borderPosition.x() == Broodwar->mapWidth() - 1) {
		return Border_Right;
	} else if (borderPosition.y() == Broodwar->mapHeight() - 1) {
		return Border_Bottom;
	}

	return Border_Lim;
}

bool bats::areBordersNeighbors(bats::Borders borderOne, bats::Borders borderTwo) {
	for (int i = 0; i < 2; ++i) {
		// Left - Top
		if (borderOne == Border_Left && borderTwo == Border_Top) {
			return true;
		}
		// Left - Bottom
		else if (borderOne == Border_Left && borderTwo == Border_Bottom) {
			return true;
		}
		// Right - Top
		else if (borderOne == Border_Right && borderTwo == Border_Top) {
			return true;
		}
		// Right - Bottom
		else if (borderOne == Border_Right && borderTwo == Border_Bottom) {
			return true;
		}

		// Swap positions to test the other order too
		std::swap(borderOne, borderTwo);
	}

	return false;
}

TilePosition bats::getCorner(bats::Borders borderOne, bats::Borders borderTwo) {
	// Left - Top
	if ((borderOne == Border_Left && borderTwo == Border_Top) ||
		(borderOne == Border_Top && borderTwo == Border_Left))
	{
		return TilePosition(0, 0);
	}
	// Left - Bottom
	else if ((borderOne == Border_Left && borderTwo == Border_Bottom) ||
			(borderOne == Border_Bottom && borderTwo == Border_Left))
	{
		return TilePosition(0, Broodwar->mapHeight() -1);
	}
	// Right - Top
	else if ((borderOne == Border_Right && borderTwo == Border_Top) ||
			(borderOne == Border_Top && borderTwo == Border_Right))
	{
		return TilePosition(Broodwar->mapWidth() -1, 0);
	}
	// Right - Bottom
	else if ((borderOne == Border_Right && borderTwo == Border_Bottom) ||
		(borderOne == Border_Bottom && borderTwo == Border_Right))
	{
		return TilePosition(Broodwar->mapWidth() -1, Broodwar->mapHeight() -1);
	}

	return TilePositions::Invalid;
}

std::ostream& bats::operator<<(std::ostream& out, const TilePosition& position) {
	out << "(" << position.x() << ", " << position.y() << ")";
	return out;
}

/**
 * \copydoc operator<<(std::ostream&,BWPAI::Tileposition&)
 */
std::ostream& bats::operator<<(std::ostream& out, const Position& position) {
	out << "(" << position.x() << ", " << position.y() << ")";
	return out;
}