#include "SquadManager.h"
#include "Squad.h"

using namespace bats;
using std::tr1::shared_ptr;
using std::tr1::const_pointer_cast;

SquadManager* SquadManager::mpsInstance = NULL;

SquadManager::SquadManager() {
	// Does nothing
}

SquadManager::~SquadManager() {
	// Using shared pointer so no need deleting squads :)

	mpsInstance = NULL;
}

SquadManager* SquadManager::getInstance() {
	if (mpsInstance == NULL) {
		mpsInstance = new SquadManager();
	}
	return mpsInstance;
}

SquadPtr SquadManager::getSquad(SquadId squadId) {
	const SquadCstPtr constPtr = const_cast<const SquadManager*>(this)->getSquad(squadId);
	const SquadPtr squad = const_pointer_cast<Squad>(constPtr);
	return squad;
}

SquadCstPtr SquadManager::getSquad(SquadId squadId) const {
	std::map<SquadId, SquadPtr>::const_iterator foundSquad;
	foundSquad = mSquads.find(squadId);
	if (foundSquad != mSquads.end()) {
		const SquadCstPtr& constPtr = const_pointer_cast<const Squad>(foundSquad->second);
		return constPtr;
	} else {
		ERROR_MESSAGE(false, "Could not find squad with squad id: "	<< squadId);
		return SquadPtr();
	}
}

SquadIt SquadManager::begin() {
	return mSquads.begin();
}

SquadCstIt SquadManager::begin() const {
	return mSquads.begin();
}

SquadIt SquadManager::end() {
	return mSquads.end();
}

SquadCstIt SquadManager::end() const {
	return mSquads.end();
}

void SquadManager::computeActions() {
	std::map<SquadId, SquadPtr>::iterator squadIt = mSquads.begin();
	while(squadIt != mSquads.end()) {
		if (!squadIt->second->isEmpty()) {
			// Only compute squads that aren't inactive
			if (squadIt->second->getState() != Squad::State_Inactive) {
				squadIt->second->computeActions();
			}
			++squadIt;
		}
		// Delete empty squads
		else {
			squadIt = mSquads.erase(squadIt);
		}	
	}
}

void SquadManager::addSquad(const SquadPtr& pSquad) {
	assert(pSquad != NULL);
	mSquads[pSquad->getSquadId()] = pSquad;
}

void SquadManager::removeSquad(const SquadId& squadId) {
	assert(squadId != SquadId::INVALID_KEY);
	size_t cErased = mSquads.erase(squadId);
	if (cErased == 0) {
		ERROR_MESSAGE(false, "Could not find the squad to erase with id: " << squadId);
	}
}

void SquadManager::printGraphicDebugInfo() {
	std::map<SquadId, SquadPtr>::iterator squadIt;
	for (squadIt = mSquads.begin(); squadIt != mSquads.end(); ++squadIt) {
		squadIt->second->printGraphicDebugInfo();
	}
}