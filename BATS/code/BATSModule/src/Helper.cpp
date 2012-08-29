#include "Helper.h"
#include <ostream>
#include <BWAPI/Game.h>
#include <BWAPI/Constants.h>
#include <set>
#include <cfloat>

using namespace BWAPI;
using namespace bats;
using namespace std;

std::string bats::TextColors::LIGHT_BLUE = "\x02";
std::string bats::TextColors::DARK_YELLOW = "\x03";
std::string bats::TextColors::WHITE = "\x04";
std::string bats::TextColors::DARK_GREY = "\x05";
std::string bats::TextColors::DARK_RED = "\x06";
std::string bats::TextColors::GREEN = "\x07";
std::string bats::TextColors::RED = "\x08";
std::string bats::TextColors::BLUE = "\x0E";
std::string bats::TextColors::TEAL = "\x0F";
std::string bats::TextColors::PURPLE = "\x10";
std::string bats::TextColors::ORANGE = "\x11";
std::string bats::TextColors::BROWN = "\x15";
std::string bats::TextColors::LIGHT_GREY = "\x16";
std::string bats::TextColors::YELLOW = "\x17";
std::string bats::TextColors::DARK_GREEN = "\x18";
std::string bats::TextColors::LIGHT_YELLOW = "\x19";
std::string bats::TextColors::PALE_PINK = "\x1B";
std::string bats::TextColors::ROYAL_BLUE = "\x1C";
std::string bats::TextColors::GREY_GREEN = "\x1D";
std::string bats::TextColors::GREY_BLUE = "\x1E";
std::string bats::TextColors::CYAN = "\x1F";

BWAPI::TilePosition bats::getClosestBorder(const BWAPI::TilePosition& position) {
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

bats::Borders bats::getAtWhichBorder(const BWAPI::TilePosition& borderPosition) {
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

BWAPI::TilePosition bats::getCorner(bats::Borders borderOne, bats::Borders borderTwo) {
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

std::ostream& bats::operator<<(std::ostream& out, const BWAPI::TilePosition& position) {
	out << "(" << position.x() << ", " << position.y() << ")";
	return out;
}

std::ostream& bats::operator<<(std::ostream& out, const BWAPI::Position& position) {
	out << "(" << position.x() << ", " << position.y() << ")";
	return out;
}