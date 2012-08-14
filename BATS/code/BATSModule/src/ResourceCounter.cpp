#include "ResourceCounter.h"
#include "ResourceGroup.h"
#include "Resource.h"
#include "Config.h"
#include "Helper.h"
#include "Utilities/Logger.h"
#include "BTHAIModule/Source/Profiler.h"
#include <cstdlib> // For NULL
#include <BWAPI/Game.h>
#include <BWTA.h>

using namespace bats;
using namespace BWAPI;
using namespace BWTA;
using namespace std;

ResourceCounter* ResourceCounter::msInstance = NULL;

ResourceCounter::ResourceCounter() {
	mFrameLastCall = Broodwar->getFrameCount();
	
	addResourceGroups();
}

ResourceCounter::~ResourceCounter() {
	msInstance = NULL;
}

ResourceCounter* ResourceCounter::getInstance() {
	if (NULL == msInstance) {
		msInstance = new ResourceCounter();
	}
	return msInstance;
}

ResourceGroupCstIt ResourceCounter::begin() const {
	return mGroupsById.begin();
}

ResourceGroupCstIt ResourceCounter::end() const {
	return mGroupsById.end();
}

ResourceGroupCstPtr ResourceCounter::getResourceGroup(int groupId) const {
	const ResourceGroupCstIt& groupIt = mGroupsById.find(groupId);
	if (groupIt != mGroupsById.end()) {
		return groupIt->second;
	} else {
		return ResourceGroupCstPtr();
	}
}

ResourceGroupCstPtr ResourceCounter::getResourceGroup(const BWAPI::TilePosition& expansionPosition) const {
	const std::map<TilePosition, ResourceGroupPtr>::const_iterator& groupIt =
		mGroupsByPosition.find(expansionPosition);

	if (groupIt != mGroupsByPosition.end()) {
		return groupIt->second;
	} else {
		return ResourceGroupCstPtr();
	}
}

ResourceGroupCstPtr ResourceCounter::getClosestResourceGroup(const BWAPI::TilePosition& position) const {
	int bestDist = INT_MAX;
	ResourceGroupPtr bestResource;

	for (ResourceGroupCstIt resourceIt = mGroupsById.begin(); resourceIt != mGroupsById.end(); ++resourceIt) {
		const TilePosition& expansionPos = resourceIt->second->getExpansionPosition();
		int dist = bats::getSquaredDistance(expansionPos, position);
		if (dist < bestDist) {
			bestDist = dist;
			bestResource = resourceIt->second;
		}
	}

	return bestResource;
}

void ResourceCounter::update() {
	// Don't update to often.
	int cFrame = Broodwar->getFrameCount();
	if (cFrame - mFrameLastCall < config::frame_distribution::RESOURCE_COUNTER) {
		return;
	}
	mFrameLastCall = cFrame;

	Profiler::getInstance()->start("ResourceCounter::update()");

	// Update all existing first
	ResourceGroupIt groupIt;
	for (groupIt = mGroupsById.begin(); groupIt != mGroupsById.end(); ++groupIt) {
		groupIt->second->update();
	}

	/// @todo If necessary, split the workload in two. updating resource one frame,
	/// adding another frame.

	// Add new resources that have been found. We only use minerals now
	const std::set<Unit*>& minerals = Broodwar->getMinerals();
	std::set<Unit*>::const_iterator mineralIt;
	for (mineralIt = minerals.begin(); mineralIt != minerals.end(); ++mineralIt) {
		// Get the resource group
		ResourceGroupIt groupIt;
		groupIt = mGroupsById.find((*mineralIt)->getResourceGroup());

		if (groupIt != mGroupsById.end()) {
			// Add resource if it does not already exist
			if (groupIt->second->hasResource((*mineralIt)->getID()) == false) {
				groupIt->second->addResource((*mineralIt));
			}
		} else {
			ERROR_MESSAGE(false, "Could not find the resource group (" <<
				(*mineralIt)->getResourceGroup() << ")!"
			);
		}
	}

	Profiler::getInstance()->end("ResourceCounter::update()");
}

void ResourceCounter::addResourceGroups() {
	// Get all expansion points
	const set<BaseLocation*>& baseLocations = BWTA::getBaseLocations();
	set<BaseLocation*>::const_iterator baseIt;
	for (baseIt = baseLocations.begin(); baseIt != baseLocations.end(); ++baseIt) {
		BaseLocation* pCurrentBase = (*baseIt);

		// Get all minerals to find out the resource group id
		const set<Unit*>& minerals = pCurrentBase->getStaticMinerals();

		if (!minerals.empty()) {

			Unit* firstMineral = *minerals.begin();
			int resourceGroupId = firstMineral->getResourceGroup();
		
			// Create Resource group for the base
			ResourceGroup* pNewResourceGroup = new ResourceGroup(
				pCurrentBase->getTilePosition(),
				resourceGroupId
			);
			ResourceGroupPtr resourceGroup(pNewResourceGroup);

			// Add to maps
			mGroupsById.insert(make_pair(resourceGroupId,resourceGroup));
			mGroupsByPosition.insert(make_pair(resourceGroup->getExpansionPosition(), resourceGroup));

			DEBUG_MESSAGE(utilities::LogLevel_Finest, "Resource group added: " << resourceGroupId);
		} else {
			ERROR_MESSAGE(false, "Did not find any minerals for the current Baselocation (" <<
				pCurrentBase->getTilePosition().x() << ", " <<
				pCurrentBase->getTilePosition().y() << ")!"
			);
		}


		/// @todo add vespene geysers?
	}
}