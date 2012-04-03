#pragma once

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
 * Returns the squared distance between two points
 * @pre type T needs to have a function x() and y()
 * @pre neither a or b shall be a pointer.
 * @param a the first point
 * @param b the second point
 * @return squared distance between point a and b.
 */
template<typename T>
inline double getSquaredDistance(const T& a, const T& b) {
	T diffDistance;
	diffDistance.x() = a.x() - b.x();
	diffDistance.y() = a.y() - b.y();
	return diffDistance.x() * diffDistance.x() + diffDistance.y() * diffDistance.y();
}

}