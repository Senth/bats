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

shared_ptr<Squad> SquadManager::getSquad(const SquadId& squadId) {
	const shared_ptr<const Squad> constPtr = const_cast<const SquadManager*>(this)->getSquad(squadId);
	const shared_ptr<Squad> squad = const_pointer_cast<Squad>(constPtr);
	return squad;
}

shared_ptr<const Squad> SquadManager::getSquad(const SquadId& squadId) const {
	std::map<SquadId, shared_ptr<Squad>>::const_iterator foundSquad;
	foundSquad = mSquads.find(squadId);
	if (foundSquad != mSquads.end()) {
		const shared_ptr<const Squad>& constPtr = const_pointer_cast<const Squad>(foundSquad->second);
		return constPtr;
	} else {
		ERROR_MESSAGE(false, "Could not find squad with squad id: "	<< squadId);
		return shared_ptr<Squad>();
	}
}

std::map<SquadId, shared_ptr<Squad>>::iterator SquadManager::begin() {
	return mSquads.begin();
}

std::map<SquadId, shared_ptr<Squad>>::const_iterator SquadManager::begin() const {
	return mSquads.begin();
}

std::map<SquadId, shared_ptr<Squad>>::iterator SquadManager::end() {
	return mSquads.end();
}

std::map<SquadId, shared_ptr<Squad>>::const_iterator SquadManager::end() const {
	return mSquads.end();
}

void SquadManager::computeActions() {
	std::map<SquadId, shared_ptr<Squad>>::iterator squadIt = mSquads.begin();
	while(squadIt != mSquads.end()) {
		if (!squadIt->second->isEmpty()) {
			squadIt->second->computeActions();
			++squadIt;
		}
		// Delete empty squads
		else {
			squadIt = mSquads.erase(squadIt);
		}	
	}
}

void SquadManager::addSquad(const shared_ptr<Squad>& pSquad) {
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