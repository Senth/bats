#include "DefenseManager.h"
#include "UnitManager.h"
#include "SquadManager.h"
#include "DefenseMoveSquad.h"
#include <BWAPI/Unit.h>
#include <BWTA/Region.h>
#include <BWTA/Chokepoint.h>
#include <cstdlib> // For NULL

using namespace bats;
using namespace BWAPI;
using namespace std;

DefenseManager* DefenseManager::mpsInstance = NULL;

DefenseManager::DefenseManager() {
	mpUnitManager = NULL;

	mpUnitManager = UnitManager::getInstance();
}

DefenseManager::~DefenseManager() {
	mpsInstance = NULL;
}

DefenseManager* DefenseManager::getInstance() {
	if (NULL == mpsInstance) {
		mpsInstance = new DefenseManager();
	}
	return mpsInstance;
}

void DefenseManager::update() {
	updateDefendPositions();


	// Get all free units, to be added to the defense squads.
	/// @todo Remove all overlords
	vector<UnitAgent*> freeUnits = mpUnitManager->getUnitsByFilter(UnitFilter_HasNoSquad);

	// Remove transportations
	vector<UnitAgent*>::const_iterator unitIt = freeUnits.begin();
	while (unitIt != freeUnits.end()) {
		if ((*unitIt)->isTransport()) {
			unitIt = freeUnits.erase(unitIt);
		} else {
			++unitIt;
		}
	}

	// Add free units to the defense squads, except transportations
	DefenseMoveSquadPtr activeMoveSquad;
	const vector<DefenseMoveSquadPtr>& moveSquads = mpSquadManager->getSquads<DefenseMoveSquad>();
	if (moveSquads.empty()) {
		DefenseMoveSquad* pMoveSquad = new DefenseMoveSquad(freeUnits);
		activeMoveSquad = pMoveSquad->getThis();
	} else {
		// We will always have only one DefenseMoveSquad
		activeMoveSquad = moveSquads[0];
		activeMoveSquad->addUnits(freeUnits);
	}

	/// @todo Set new wait position for the DefenseMoveSquad to the middle of all defending points
	

	/// @todo Remove any DefenseHoldSquad in position that shall not be defended any longer.

	/// @todo is any defense point undefended, create DefenseHoldSquad for that position
	
	/// @todo Does any DefenseHoldSquad need more units, i.e. not full.

	/// @todo Can we create higher priority composition for any defended area?
}

vector<Unit*> DefenseManager::getAllFreeUnits() {
	/// @todo stub
	return vector<Unit*>();
}

bool DefenseManager::isUnderAttack() const {
	/// @todo stub
	return false;
}

void DefenseManager::updateDefendPositions() {
	vector<BWTA::Chokepoint*> defendChokepoints = getDefendChokepoints();
	
	// Find a good position to defend the choke position
}

bool DefenseManager::isRegionOccupiedByOurTeam(BWTA::Region* pRegion) {
	if (NULL == pRegion) {
		return false;
	}

	set<Player*> teamPlayers = Broodwar->allies();
	teamPlayers.insert(Broodwar->self());

	const Position regionToCheckPos = pRegion->getCenter();

	set<Player*>::const_iterator playerIt;
	for (playerIt = teamPlayers.begin(); playerIt != teamPlayers.end(); ++playerIt) {
		const set<Unit*>& units = (*playerIt)->getUnits();
		set<Unit*>::const_iterator unitIt;
		for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
			Unit* pUnit = (*unitIt);
			if (pUnit->getType().isBuilding()) {
				BWTA::Region* pUnitRegion = BWTA::getRegion(pUnit->getPosition());
				const Position& unitRegionPos = pUnitRegion->getCenter();
				
				if (unitRegionPos == regionToCheckPos) {
					return true;
				}
			}
		}
	}

	return false;
}

bool DefenseManager::isChokepointEdge(BWTA::Chokepoint* pChokepoint) {
	const pair<BWTA::Region*, BWTA::Region*>& abutRegions = pChokepoint->getRegions();
	BWTA::Region* pOccupiedRegion = NULL;
	BWTA::Region* pEmptyRegion = NULL;

	if (isRegionOccupiedByOurTeam(abutRegions.first)) {
		pOccupiedRegion = abutRegions.first;
	} else {
		pEmptyRegion = abutRegions.first;
	}

	// Now the second region needs to be inverse of the first region.
	// E.g. first -> occupied, second -> empty.
	if (isRegionOccupiedByOurTeam(abutRegions.second)) {
		if (NULL == pOccupiedRegion) {
			pOccupiedRegion = abutRegions.second;
		} else {
			return false;
		}
	} else {
		if (NULL == pEmptyRegion) {
			pEmptyRegion = abutRegions.second;
		} else {
			return false;
		}
	}
	

	// The empty region needs to abut another empty region.
	const set<BWTA::Region*> emptyNeighbors = pEmptyRegion->getReachableRegions();
	set<BWTA::Region*>::const_iterator regionIt;
	for (regionIt = emptyNeighbors.begin(); regionIt != emptyNeighbors.end(); ++regionIt) {
		// Don't check already checked region
		if ((*regionIt) == pOccupiedRegion) {
			continue;
		}

		if (!isRegionOccupiedByOurTeam(*regionIt)) {
			return true; // RETURN
		}
	}

	return false;
}

vector<BWTA::Chokepoint*> DefenseManager::getDefendChokepoints() {
	vector<BWTA::Chokepoint*> defendChokepoints;
	const set<BWTA::Chokepoint*>& chokepoints = BWTA::getChokepoints();
	set<BWTA::Chokepoint*>::const_iterator chokeIt;
	for (chokeIt = chokepoints.begin(); chokeIt != chokepoints.end(); ++chokeIt) {
		if (isChokepointEdge(*chokeIt)) {
			defendChokepoints.push_back(*chokeIt);
		}
	}

	return defendChokepoints;
}

#pragma warning(push)
#pragma warning(disable:4100)
TilePosition DefenseManager::getDefendPosition(BWTA::Chokepoint* pChokepoint) {
	/// @todo

	return TilePositions::Invalid;
}
#pragma warning(pop)