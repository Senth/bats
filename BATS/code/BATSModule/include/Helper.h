#pragma once

#include <ostream>
#include <BWAPI/Position.h>
#include <BWAPI/TilePosition.h>

namespace bats {

namespace TextColors {
	extern std::string LIGHT_BLUE;
	extern std::string DARK_YELLOW;
	extern std::string WHITE;
	extern std::string DARK_GREY;
	extern std::string DARK_RED;
	extern std::string GREEN;
	extern std::string RED;
	extern std::string BLUE;
	extern std::string TEAL;
	extern std::string PURPLE;
	extern std::string ORANGE;
	extern std::string BROWN;
	extern std::string LIGHT_GREY;
	extern std::string YELLOW;
	extern std::string DARK_GREEN;
	extern std::string LIGHT_YELLOW;
	extern std::string PALE_PINK;
	extern std::string ROYAL_BLUE;
	extern std::string GREY_GREEN;
	extern std::string GREY_BLUE;
	extern std::string CYAN;
}

/**
 * Returns the squared distance between two points. This function is faster
 * than using a regular getDistance() since getDistance() uses square root.
 * @pre type T needs to have a function x() and y()
 * @pre neither a or b shall be a pointer.
 * @param a the first point
 * @param b the second point
 * @return squared distance between point a and b.
 */
template<typename T>
inline int getSquaredDistance(const T& a, const T& b) {
	T diffDistance;
	diffDistance.x() = a.x() - b.x();
	diffDistance.y() = a.y() - b.y();
	return diffDistance.x() * diffDistance.x() + diffDistance.y() * diffDistance.y();
}

/**
 * Checks if the two positions is within the specified range.
 * @pre type T needs to have a function x() and y()
 * @param a the first point
 * @param b the second point
 * @param range the maximum distance between point a and b.
 * @param squared if range is already squared, defaults to false
 * @return true if a and b is withing range, false otherwise.
 */
template<typename T>
inline bool isWithinRange(const T& a, const T& b, int range, bool squared = false) {
	if (squared) {
		return getSquaredDistance(a, b) <= range;
	} else {
		return getSquaredDistance(a, b) <= range * range;
	}
}

/**
 * Returns the closest border position from the specified position
 * @param position the position we want to find the border from
 * @return closest border position.
 */
BWAPI::TilePosition getClosestBorder(const BWAPI::TilePosition& position);

/**
 * All the available borders.
 */
enum Borders {
	Border_First = 0,
	Border_Left = Border_First,
	Border_Top,
	Border_Right,
	Border_Bottom,
	Border_Lim
};

/**
 * Returns what border the position lies in. If the border is a border to two sides
 * it will only return one of them. Meaning if it's in the lower left corner it will either
 * return Border_Left or Border_Bottom.
 * @param borderPosition the position to test which border it belongs to.
 * @return what border the position lies in, if it doesn't lie in a border it will return
 * Border_Lim
 */
Borders getAtWhichBorder(const BWAPI::TilePosition& borderPosition);

/**
 * Returns true if the borders are neighbors. Meaning left and bottom, left and top,
 * right and bottom, and right and top.
 * @param borderOne one border
 * @param borderTwo the other border to test
 * @return true if the borders are neighbors.
 */
bool areBordersNeighbors(Borders borderOne, Borders borderTwo);

/**
 * Returns the corner for the specified borders
 * @param borderOne one border to test
 * @param borderTwo the other border to test
 * @return position of the corner these borders intersect. TilePositions::Invalid if
 * one of the borders aren't a border or if they aren't neighbors.
 */
BWAPI::TilePosition getCorner(Borders borderOne, Borders borderTwo);

/**
 * Prints out the x and y value to an outstream
 * @param out ostream object to print to
 * @param position the position to print
 * @return a reference to the ostream object
 */
std::ostream& operator<<(std::ostream& out, const BWAPI::TilePosition& position);

/**
 * \copydoc operator<<(std::ostream&,const BWAPI::TilePosition&)
 */
std::ostream& operator<<(std::ostream& out, const BWAPI::Position& position);


/**
 * Class to compare the second variable in a pair.
 */
template<typename Object, typename Compare>
struct PairCompareSecond
{
public:
	/**
	 * Returns which second is less in a pair of variables. Used for std::sort
	 * @param lhs left hand of the, i.e. should be less if returned true
	 * @param rhs right hand
	 * @return true if lhs < rhs.
	 * @pre container contain a pair where the second variable is the one that is compared.
	 */
	bool operator()(const std::pair<Object, Compare>& lhs, const std::pair<Object, Compare>& rhs)
	{
		return lhs.second < rhs.second;
	}
};
}