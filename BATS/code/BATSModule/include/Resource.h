#pragma once

#include <BWAPI/TilePosition.h>

// Forward declarations
namespace BWAPI {
	class Unit;
}

// Namespace for the project
namespace bats {

class ResourceGroup;

/**
 * A container for a resource unit that can be located anywhere on the map.
 * Does not store the unit, but rather updates the information whenever the unit
 * is visible.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class Resource {
public:
	/**
	 * Destructor
	 */
	virtual ~Resource();

	/**
	 * Returns how many resources there is left. This information is not accurate,
	 * it will only get updated once the resource unit is visible.
	 * @return resources left.
	 */
	int getCurrent() const;

	/**
	 * Returns the initial number of resources.
	 * @return initial number of resources.
	 */
	int getInitial() const;

	/**
	 * Returns the unit id of the resource.
	 * @return unit id of the resource.
	 */
	int getId() const;

	/**
	 * Returns the position of the resource.
	 * @return position of the resource.
	 */
	const BWAPI::TilePosition& getPosition() const;

	/**
	 * Tries to update the information of the Resource. This will only
	 * succeed if the resource is visible.
	 */
	void update();

	/**
	 * Assignment operator, copying shall still be possible for other classes.
	 * @param resource the resource to assign
	 * @return reference to this resource
	 */
	Resource& operator=(const Resource& resource);

private:
	/**
	 * Creates a new resource bound to the specified unit.
	 * Private only ResourceGroup can create Resources
	 * @pre pUnit is not NULL
	 * @pre pUnit is a resource unit.
	 * @param pUnit the resource unit.
	 */
	Resource(BWAPI::Unit* pUnit);

	int mCurrent;
	const int M_UNIT_ID;
	const int M_INITIAL;
	const BWAPI::TilePosition M_POSITION;

	friend class ResourceGroup;
};
}