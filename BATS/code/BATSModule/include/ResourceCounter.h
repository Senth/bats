#pragma once

#include "ResourceDefs.h"
#include <map>
#include <BWAPI/TilePosition.h>

// Namespace for the project
namespace bats {

/**
 * Tracks and counts the current number of resources all over the map.
 * The information from the ResourceCounter cannot be fully trusted since we don't
 * have full map vision. Thus it will only update individual resources once they are visible.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class ResourceCounter {
public:
	/**
	 * Destructor
	 */
	virtual ~ResourceCounter();

	/**
	 * Returns the instance of ResourceCounter.
	 * @return instance of ResourceCounter.
	 */
	static ResourceCounter* getInstance();

	/**
	 * Returns the beginning of an iterator to all ResourceGroups on the map.
	 * Since all resource groups are created from the start it will return even
	 * those that have never been visited.
	 * @return iterator pointing to the beginning of the list. The iterator is a const
	 * thus it cannot change values of the ResourceGroup
	 * @see end() to get the end of the list.
	 */
	ResourceGroupCstIt begin() const;

	/**
	 * Returns the end of an iterator to all ResourceGroups on the map.
	 * Since all resources groups are created from the start it will return even
	 * those that have never been visited.
	 * @return iterator pointing to the end of the list. The iterator is a const,
	 * thus it cannot change values of the ResourceGroup
	 * @see begin() to get the beginning of the list.
	 */
	ResourceGroupCstIt end() const;

	/**
	 * Returns the resource group with the specified resource group id.
	 * @param groupId the resource group id of the resource.
	 * @return resource group with the specified resource group id. NULL if not exists.
	 */
	ResourceGroupCstPtr getResourceGroup(int groupId) const;

	/**
	 * Returns the resource group for the specified expansion position.
	 * @param expansionPosition the expansion position of the resource group.
	 * @return resource group with the specified expansion position. NULL if no resource
	 * group exist on that position.
	 */
	ResourceGroupCstPtr getResourceGroup(const BWAPI::TilePosition& expansionPosition) const;

	/**
	 * Finds the closest resource group to the specified position
	 * @param position the position to search from
	 * @return closest resource group from the specified position
	 */
	ResourceGroupCstPtr getClosestResourceGroup(const BWAPI::TilePosition& position) const;
	  

	/**
	 * Updates all the resource groups and resources. It will not update every frame
	 * and can be configured in the config file, section: frame_distribution, variable name:
	 * resource_counter
	 */
	void update();

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	ResourceCounter();

	/**
	 * Adds all the resource groups.
	 */
	void addResourceGroups();

	int mFrameLastCall;
	std::map<int, ResourceGroupPtr> mGroupsById;
	/** Used for faster searching when we want a resource group from the specified
	 * expansion position. It has exactly the same resource groups as mGroupsById
	 * and thus needs not to be updated. */
	std::map<BWAPI::TilePosition, ResourceGroupPtr> mGroupsByPosition;

	static ResourceCounter* msInstance;
};
}