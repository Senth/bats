#pragma once

#include <ostream>
#include <BWAPI/Position.h>
#include <BWAPI/TilePosition.h>

namespace bats {

/**
 * A macro to check if something (e.g. unit) belongs to us
 * @param what does this belong to us?
 * @return true if the what belongs to us, else false
 * @pre what needs to return a BWAPI::Player by the function getPlayer.
 * @pre what needs to be a pointer.
 * @pre a global variable 'Broodwar' should be set.
 */
#define OUR(what) (what->getPlayer()->getID() == Broodwar->self()->getID())

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
 * Returns the approximate distance between two positions. This function only uses one
 * multiplication 
 */
template<typename T>
inline double getApproxDistance(const T& a, const T& b) {
	T diffDistance;
	//@todo Implement this function.
	diffDistance.x() = 0;
	diffDistance.y() = 0;
	return diffDistance;
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
 * @param position the position to test which border it belongs to.
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
 * \copydoc operator<<(std::ostream&,const BWPAI::Tileposition&)
 */
std::ostream& operator<<(std::ostream& out, const BWAPI::Position& position);
}