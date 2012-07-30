#include "SquadManager.h"
#include "Squad.h"
#include "AttackSquad.h"
#include "BTHAIModule/Source/Profiler.h"

using namespace bats;
using namespace std::tr1;

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

void SquadManager::update() {
	Profiler::getInstance()->start("SquadManager::update()");

	std::map<SquadId, SquadPtr>::iterator squadIt = mSquads.begin();
	while(squadIt != mSquads.end()) {
		if (!squadIt->second->isEmpty()) {
			// Only compute squads that aren't inactive
			if (squadIt->second->getState() != Squad::State_Inactive) {
				squadIt->second->update();
			}
			++squadIt;
		}
		// Delete empty squads
		else {
			squadIt = mSquads.erase(squadIt);
		}	
	}

	Profiler::getInstance()->end("SquadManager::update()");
}

void SquadManager::addSquad(SquadRef pSquad) {
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

AttackSquadPtr SquadManager::getFrontalAttack() {
	// Use const version
	AttackSquadCstPtr attackSquad = const_cast<const SquadManager *>(this)->getFrontalAttack();
	return const_pointer_cast<AttackSquad>(attackSquad);
}

AttackSquadCstPtr SquadManager::getFrontalAttack() const {
	for (SquadCstIt squadIt = mSquads.begin(); squadIt != mSquads.end(); ++squadIt) {
		AttackSquadPtr attackSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);
		if (NULL != attackSquad && attackSquad->isFrontalAttack()) {
			return attackSquad;
		}
	}

	return AttackSquadCstPtr();
}

std::vector<AttackSquadPtr> SquadManager::getDistractingAttacks() {
	// Use const version
	const std::vector<AttackSquadCstPtr>& distractingSquads = const_cast<const SquadManager*>(this)->getDistractingAttacks();
	return *reinterpret_cast<std::vector<AttackSquadPtr>*>(const_cast<std::vector<AttackSquadCstPtr>*>(&distractingSquads));
}

std::vector<AttackSquadCstPtr> SquadManager::getDistractingAttacks() const {
	std::vector<AttackSquadCstPtr> distractingSquads;

	for (SquadCstIt squadIt = mSquads.begin(); squadIt != mSquads.end(); ++squadIt) {
		AttackSquadPtr attackSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);
		if (NULL != attackSquad && attackSquad->isDistracting()) {
			distractingSquads.push_back(attackSquad);
		}
	}

	return distractingSquads;
}