#include "SquadManager.h"
#include "Squad.h"

using namespace bats;

SquadManager* SquadManager::mpsInstance = NULL;

SquadManager::SquadManager() {
	// Does nothing
}

SquadManager::~SquadManager() {
	// Remove all squads
	std::map<SquadId, Squad*>::iterator squadIt;
	for (squadIt = mSquads.begin(); squadIt != mSquads.end(); ++squadIt) {
		SAFE_DELETE(squadIt->second);
	}

	mpsInstance = NULL;
}

SquadManager* SquadManager::getInstance() {
	if (mpsInstance == NULL) {
		mpsInstance = new SquadManager();
	}
	return mpsInstance;
}

Squad* SquadManager::getSquad(const SquadId& squadId) {
	std::map<SquadId, Squad*>::iterator foundSquad;
	foundSquad = mSquads.find(squadId);
	if (foundSquad != mSquads.end()) {
		return foundSquad->second;
	} else {
		DEBUG_MESSAGE(utilities::LogLevel_Warning, "SquadManager::getSquad() | Could not find " <<
			"squad with squad id: " << squadId);
		return NULL;
	}
}

Squad const * const SquadManager::getSquad(const SquadId& squadId) const {
	std::map<SquadId, Squad*>::const_iterator foundSquad;
	foundSquad = mSquads.find(squadId);
	if (foundSquad != mSquads.end()) {
		return foundSquad->second;
	} else {
		ERROR_MESSAGE(false, "Could not find squad with squad id: "	<< squadId);
		return NULL;
	}
}

std::map<SquadId, Squad*>::iterator SquadManager::begin() {
	return mSquads.begin();
}

std::map<SquadId, Squad*>::const_iterator SquadManager::begin() const {
	return mSquads.begin();
}

std::map<SquadId, Squad*>::iterator SquadManager::end() {
	return mSquads.end();
}

std::map<SquadId, Squad*>::const_iterator SquadManager::end() const {
	return mSquads.end();
}

void SquadManager::computeActions() {
	std::map<SquadId, Squad*>::iterator squadIt;
	for (squadIt = mSquads.begin(); squadIt != mSquads.end(); ++squadIt) {
		squadIt->second->computeActions();
	}
}

void SquadManager::addSquad(Squad* pSquad) {
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