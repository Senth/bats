#pragma once

#include <BWAPI/TilePosition.h>
#include <map>
#include "Resource.h"

// Forward declarations
namespace BWAPI {
	class Unit;
}

// Namespace for the project
namespace bats {

/**
 * Groups together a bunch of resources located close to each other. This is done
 * by checking the units' resource group id. In addition the resource group is always
 * grouped together with an base location, which can be either occupied or vacant.
 * 
 * The ResourceGroup does not have accurate information as the resources are only
 * updated when the resource fields are visible.
 * 
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class ResourceGroup {
public:
	/**
	 * Creates and bind the resource group together with the specified expansion
	 * position.
	 * @param expPosition position of the expansion the resource group belongs to.
	 * @param resourceGroupId the resource group id that all resources in this group
	 * belongs to.
	 */
	ResourceGroup(const BWAPI::TilePosition& expPosition, int resourceGroupId);

	/**
	 * Destructor
	 */
	virtual ~ResourceGroup();

	/**
	 * Adds a resource unit to the resource group. This resource does not need to be
	 * removed or updated. It will get updated once update() is called.
	 * @pre pUnit is not NULL.
	 * @param pUnit pointer to the 
	 */
	void addResource(BWAPI::Unit* pUnit);

	/**
	 * Checks whether this resource groups has the specified resource.
	 * @return true if the resource exist within this group
	 */
	bool hasResource(int id) const;

	/**
	 * Returns how much resources there are left in fractions.
	 * @return how much resources there are left, in the interval [0.0,1.0].
	 * Where 0.0 means all resources have been mined out and 1.0 means no resources
	 * have been mined.
	 * Special case: When no resources have been found for this resource group yet it
	 * will treat as though no resources have been mined, thus it will return 1.0.
	 */
	double getResourcesLeft() const;

	/**
	 * Tries to update all resources in the resource group. They will only get
	 * updated if they are visible.
	 */
	void update();

	/**
	 * Assignment operator, const makes it impossible for the standard assignment
	 * operator to work.
	 * @param resourceGroup the resource group to assign.
	 * @return reference to this ResourceGroup.
	 */
	ResourceGroup& operator=(const ResourceGroup& resourceGroup);

private:

	std::map<int,Resource> mResources;

	const BWAPI::TilePosition M_EXP_POSITION;
	const int M_RESOURCE_GROUP_ID;
};
}