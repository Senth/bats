#include "ResourceCounter.h"
#include "ResourceGroup.h"
#include "Resource.h"
#include "Config.h"
#include "Utilities/Logger.h"
#include <cstdlib> // For NULL
#include <BWAPI/Game.h>
#include <BWTA.h>

using namespace bats;
using namespace BWAPI;
using namespace BWTA;
using std::map;
using std::set;
using std::tr1::shared_ptr;

ResourceCounter* ResourceCounter::mpsInstance = NULL;

ResourceCounter::ResourceCounter() {
	mFrameLastCall = Broodwar->getFrameCount();
	
	addResourceGroups();
}

ResourceCounter::~ResourceCounter() {
	mpsInstance = NULL;
}

ResourceCounter* ResourceCounter::getInstance() {
	if (NULL == mpsInstance) {
		mpsInstance = new ResourceCounter();
	}
	return mpsInstance;
}

map<int, shared_ptr<ResourceGroup>>::const_iterator ResourceCounter::begin() const {
	return mGroupsById.begin();
}

map<int, shared_ptr<ResourceGroup>>::const_iterator ResourceCounter::end() const {
	return mGroupsById.end();
}

shared_ptr<const ResourceGroup> ResourceCounter::getResourceGroup(int groupId) const {
	const map<int, shared_ptr<ResourceGroup>>::const_iterator& groupIt = mGroupsById.find(groupId);
	if (groupIt != mGroupsById.end()) {
		return groupIt->second;
	} else {
		return shared_ptr<const ResourceGroup>();
	}
}

shared_ptr<const ResourceGroup> ResourceCounter::getResourceGroup(const BWAPI::TilePosition& expansionPosition) {
	const map<TilePosition, shared_ptr<ResourceGroup>>::const_iterator& groupIt =
		mGroupsByPosition.find(expansionPosition);

	if (groupIt != mGroupsByPosition.end()) {
		return groupIt->second;
	} else {
		return shared_ptr<const ResourceGroup>();
	}
}

void ResourceCounter::update() {
	// Don't update to often.
	int cFrame = Broodwar->getFrameCount();
	if (cFrame - mFrameLastCall < config::frame_distribution::RESOURCE_COUNTER) {
		return;
	}
	mFrameLastCall = cFrame;


	// Update all existing first
	map<int, shared_ptr<ResourceGroup>>::iterator groupIt;
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
		map<int, shared_ptr<ResourceGroup>>::iterator groupIt;
		groupIt = mGroupsById.find((*mineralIt)->getResourceGroup());

		if (groupIt != mGroupsById.end()) {
			// Add resource if it does not already exist
			if (groupIt->second->hasResource((*mineralIt)->getID()) == false) {
				groupIt->second->addResource((*mineralIt));
			}
		} else {
			ERROR_MESSAGE(false, "Could not find the resource group, this shall never happen!");
		}
	}
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
			shared_ptr<ResourceGroup> resourceGroup = shared_ptr<ResourceGroup>(pNewResourceGroup);

			// Add to maps
			mGroupsById.insert(std::pair<int, shared_ptr<ResourceGroup>>(
				resourceGroupId,
				resourceGroup
			));
			mGroupsByPosition.insert(std::pair<TilePosition, shared_ptr<ResourceGroup>>(
				resourceGroup->getExpansionPosition(),
				resourceGroup
			));
		} else {
			ERROR_MESSAGE(false, "Did not find any minerals for the current Baselocation (" <<
				pCurrentBase->getTilePosition().x() << ", " <<
				pCurrentBase->getTilePosition().y() << ")!"
			);
		}
	}
}