#include <BWAPI/TilePosition.h>
#include <BWTA/Region.h>

namespace bats {

/**
 * A region we have visited, and when we visited it.
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 * Migrated to own header and source file. Added expansion and other functionality
 */
class ExploreData {
public:
	/**
	 * Creates an exploration point for the specified position
	 * @param position the position of the exploration point
	 * @param bExpansion if the position is an expansion, defaults to false
	 */
	ExploreData(const BWAPI::TilePosition& position, bool bExpansion = false);

	/**
	 * Virtual destructor
	 */
	virtual ~ExploreData();

	/**
	 * Returns true if the time since visiting this position is less than the specified
	 * explorations data.
	 * @param rightExploreData the right side of <
	 * @return true if the time since visiting this position is less than the other.
	 */
	bool operator<(const ExploreData& rightExploreData) const;

	/**
	 * Returns the center position of the region
	 * @return center position of the region
	 */
	const BWAPI::TilePosition& getCenterPosition() const;

	/**
	 * Returns true if the position is an expansion.
	 * @return true if the position is an expansion position.
	 */
	bool isExpansion() const;

	/**
	 * Checks if the position matches the center of this region.
	 * @return true if the specified center position is the center of this region or a neighbor.
	 */
	bool matches(const BWAPI::TilePosition& center) const;

	/**
	 * Checks if the specified region matches this region.
	 * @param pRegion the region to test
	 * @return true if the regions match
	 */
	bool matches(BWTA::Region* region) const;

	/**
	 * Checks whether the specified TilePosition is within this exploration region.
	 * @pre only works for regions and not expansion positions.
	 * @param position the position we want to check whether it's within this region.
	 * @return true if the position is within this region.
	 */
	bool isWithin(const BWAPI::TilePosition& position) const;

	/**
	 * Returns how many seconds (in fastest speed) since the location was checked.
	 * @return number of seconds since last visit.
	 */
	double secondsSinceLastVisit() const;

	/**
	 * Returns the last frame this position was visited
	 * @return last frame this position was visited
	 */
	int getLastVisitFrame() const;

	/**
	 * Sets the ExploreData region as visited.
	 */
	void updateVisited();

private:
	BWAPI::TilePosition mPosition;
	int mLastVisitFrame;
	bool mbExpansion;
};
}