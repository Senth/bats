#include "ResourceGroup.h"
#include <BWAPI/Unit.h>
#include <cassert>

using namespace bats;
using namespace BWAPI;
using std::map;

ResourceGroup::ResourceGroup(const TilePosition& expPosition, int resourceGroupId) :
	M_EXP_POSITION(expPosition),
	M_RESOURCE_GROUP_ID(resourceGroupId)
{
	// Does nothing
}

ResourceGroup::~ResourceGroup() {
	// Does nothing
}

void ResourceGroup::addResource(BWAPI::Unit* pUnit) {
	assert(NULL != pUnit);

	mResources.insert(std::pair<int, Resource>(pUnit->getID(), Resource(pUnit)));
}

bool ResourceGroup::hasResource(int id) const {
	return mResources.count(id) > 1;
}

double ResourceGroup::getResourcesLeftInFraction() const {
	int initial = 0;
	int current = 0;

	map<int, Resource>::const_iterator resourceIt;
	for (resourceIt = mResources.begin(); resourceIt != mResources.end(); ++resourceIt) {
		initial += resourceIt->second.getInitial();
		current += resourceIt->second.getCurrent();
	}

	double fraction = 1.0;
	if (initial != 0) {
		fraction = static_cast<double>(current) / static_cast<double>(initial);
	}

	return fraction;
}

void ResourceGroup::update() {
	map<int, Resource>::iterator resourceIt;
	for (resourceIt = mResources.begin(); resourceIt != mResources.end(); ++resourceIt) {
		resourceIt->second.update();
	}
}

const TilePosition& ResourceGroup::getExpansionPosition() const {
	return M_EXP_POSITION;
}

int ResourceGroup::getId() const {
	return M_RESOURCE_GROUP_ID;
}

ResourceGroup& ResourceGroup::operator =(const ResourceGroup& resourceGroup) {
	mResources = resourceGroup.mResources;
	const_cast<TilePosition&>(M_EXP_POSITION) = resourceGroup.M_EXP_POSITION;
	const_cast<int&>(M_RESOURCE_GROUP_ID) = resourceGroup.M_RESOURCE_GROUP_ID;
	return *this;
}