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

// Forward declarations
class ResourceCounter;

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
	 * Destructor
	 */
	virtual ~ResourceGroup();

	/**
	 * Adds a resource unit to the resource group. This resource does not need to be
	 * removed or updated. It will get updated once update() is called.
	 * @pre unit is not NULL.
	 * @param unit pointer to the 
	 */
	void addResource(BWAPI::Unit* unit);

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
	double getResourcesLeftInFraction() const;

	/**
	 * Returns the number active mineral patches this group has. By active the resource
	 * should have more than 0 minerals left.
	 * @return number of active mineral patches this group has.
	 */
	int getActiveMineralPatchCount() const;

	/**
	 * Tries to update all resources in the resource group. They will only get
	 * updated if they are visible.
	 */
	void update();

	/**
	 * Returns the expansion position of the ResourceGroup.
	 * @return expansion position of the ResourceGroup.
	 */
	const BWAPI::TilePosition& getExpansionPosition() const;

	/**
	 * Returns the id of the resource group
	 * @return id of the resource group. Same as Unit->getResourceGroup()
	 */
	int getId() const;

	/**
	 * Assignment operator, const makes it impossible for the standard assignment
	 * operator to work.
	 * @param resourceGroup the resource group to assign.
	 * @return reference to this ResourceGroup.
	 */
	ResourceGroup& operator=(const ResourceGroup& resourceGroup);

private:
	/**
	 * Creates and binds the resource group together with the specified expansion
	 * position. Only available for ResourceCounter as it is a friend.
	 * @param expPosition position of the expansion the resource group belongs to.
	 * @param resourceGroupId the resource group id that all resources in this group
	 * belongs to.
	 */
	ResourceGroup(const BWAPI::TilePosition& expPosition, int resourceGroupId);

	friend class ResourceCounter;

	std::map<int,Resource> mResources;

	const BWAPI::TilePosition M_EXP_POSITION;
	const int M_RESOURCE_GROUP_ID;
};
}