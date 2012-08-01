#include "TransportAgent.h"
#include "PFManager.h"
#include "BatsModule\include\UnitManager.h"
#include "BatsModule\include\SquadManager.h"
#include "BatsModule\include\Squad.h"

using namespace BWAPI;
using namespace std;

TransportAgent::TransportAgent(Unit* pUnit) : UnitAgent(pUnit)
{
	agentType = "TransportAgent";

	mLoadMax = type.spaceProvided();
	mLoadQueueSpaces = 0;
	mUnloading = false;
	mLoadCached = 0;
	mLastFrameUpdate = 0;
}

int TransportAgent::getFreeLoadSpace(bool includeQueuedUnits) const {
	// Don't calculate it every frame, use cached result then
	if (mLastFrameUpdate != Broodwar->getFrameCount()) {
		mLastFrameUpdate = Broodwar->getFrameCount();
	} else {
		int load = mLoadCached;
		if (includeQueuedUnits) {
			load -= mLoadQueueSpaces;
		}
		return mLoadMax - load;
	}

	int load = 0;
	if (getSquadId() != bats::SquadId::INVALID_KEY) {
		std::tr1::shared_ptr<const bats::Squad> squad = bats::SquadManager::getInstance()->getSquad(getSquadId());

		if (NULL != squad) {	
			const vector<const UnitAgent*>& units = squad->getUnits();
			for (size_t i = 0; i < units.size(); ++i) {
				const UnitAgent* pUnitCurrent = units[i];

				if (pUnitCurrent->getUnit()->isLoaded()) {
					if (pUnitCurrent->getUnit()->getTransport()->getID() == getUnit()->getID()) {
						load += pUnitCurrent->getUnitType().spaceRequired();
					}
				}
			}
		}

		mLoadCached = load;
	}
	// Else check with all our units
	else {
		std::vector<UnitAgent*> units = bats::UnitManager::getInstance()->getUnitsByFilter();

		for (size_t i = 0; i < units.size(); ++i) {
			const UnitAgent* pUnitCurrent = units[i];

			if (pUnitCurrent->getUnit()->isLoaded()) {
				if (pUnitCurrent->getUnit()->getTransport()->getID() == getUnit()->getID()) {
					load += pUnitCurrent->getUnitType().spaceRequired();
				}
			}
		}
	}

	// If we want to include queued units we have to update the list, otherwise we might calculate
	// a unit twice
	if (includeQueuedUnits) {
		TransportAgent* pThis = const_cast<TransportAgent*>(this);
		pThis->updateQueue();
		load += mLoadQueueSpaces;
	}

	return mLoadMax - load;
}

bool TransportAgent::isValidLoadUnit(UnitAgent* pUnit)
{
	if (pUnit->getUnitType().isFlyer()) return false;
	if (pUnit->getUnit()->isLoaded()) return false;
	if (pUnit->getUnit()->isBeingConstructed()) return false;
	if (pUnit->isTransport()) return false;
	return true;
}

void TransportAgent::computeActions()
{
	if (unit->isBeingConstructed()) return;

	// Sometimes a squad might be disbanded without ordering the unload command. When this happens
	// the transportation will check, whenever it gets removed from a squad, if it has been loaded
	// and will set mUnloading to true.
	static bool firstNotSquadTime = true;

	// Only do something if we have a goal
	if (getGoal() != TilePositions::Invalid) {
		firstNotSquadTime = true;

		// Only check loading etc if we are in a squad.
		if (getSquadId() != bats::SquadId::INVALID_KEY) {
			updateQueue();

			// Loading units
			if (!mLoadQueue.empty()) {
				getUnit()->load(mLoadQueue.front()->getUnit());
			} else {
				// If we want to unload
				if (mUnloading) {
					if (getFreeLoadSpace() < mLoadMax) {
						getUnit()->unloadAll();
					} else {
						mUnloading = false;
					}
				} else {
					bool defensive = true;
					PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
				}
			}
		} else {
			// If we want to unload.
			if (mUnloading || firstNotSquadTime) {
				if (getFreeLoadSpace() < mLoadMax) {
					mUnloading = true;
					getUnit()->unloadAll();
				} else {
					mUnloading = false;
				}

				firstNotSquadTime = false;

			} else {
				bool defensive = true;
				PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
			}
		}
	}
}

bool TransportAgent::loadUnit(UnitAgent* pUnit) {
	mUnloading = false;

	// Cannot load agent
	if (!isValidLoadUnit(pUnit)) {
		return false;
	}

	// Check if we have enough space for it
	int freeSpace = getFreeLoadSpace(true);
	int unitSpace = pUnit->getUnitType().spaceRequired();
	if (freeSpace > unitSpace) {
		mLoadQueue.push_back(pUnit);
		mLoadQueueSpaces += unitSpace;
		return true;
	} else {
		return false;
	}
}

void TransportAgent::updateQueue() {
	// Don't update several times every frame
	static int sLastFrameUpdate = 0;

	if (sLastFrameUpdate != Broodwar->getFrameCount()) {
		sLastFrameUpdate = Broodwar->getFrameCount();
	} else {
		return;
	}

	vector<UnitAgent*>::iterator unitIt = mLoadQueue.begin();
	while(unitIt != mLoadQueue.end()) {
		UnitAgent* currentUnit = (*unitIt);
		if (!currentUnit->isAlive() || currentUnit->getUnit()->isLoaded()) {
			mLoadQueueSpaces -= currentUnit->getUnitType().spaceRequired();
			unitIt = mLoadQueue.erase(unitIt);
		} else {
			++unitIt;
		}
	}
}

void TransportAgent::clearLoadQueue() {
	mLoadQueueSpaces = 0;
	mLoadQueue.clear();
}

void TransportAgent::unloadAll() {
	clearLoadQueue();
	mUnloading = true;
}

bool TransportAgent::isLoading() const {
	return !mLoadQueue.empty();
}