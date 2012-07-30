#include "DefenseManager.h"
#include "UnitManager.h"
#include "SquadManager.h"
#include "DefenseMoveSquad.h"
#include "Helper.h"
#include "Config.h"
#include "ExplorationManager.h"
#include "GameTime.h"
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
	mpSquadManager = NULL;
	mpGameTime = NULL;

	mFrameCallLast = 0;
	mpUnitManager = UnitManager::getInstance();
	mpSquadManager = SquadManager::getInstance();
	mpGameTime = GameTime::getInstance();
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
	// Don't update too often
	if (mFrameCallLast + config::frame_distribution::DEFENSE_MANAGER > mpGameTime->getFrameCount()) {
		return;
	}
	mFrameCallLast = mpGameTime->getFrameCount();

	updateDefendPositions();
	updateUnderAttackPositions();
	updateMoveSquad();

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
	vector<BWTA::Chokepoint*> newDefendChokepoints = getDefendChokepoints();
	
	// Find good defend positions for all new defend choke points
	for (size_t i = 0; i < newDefendChokepoints.size(); ++i) {
		DefendMap::const_iterator foundIt =
			mDefendPositions.find(newDefendChokepoints[i]);

		if (foundIt == mDefendPositions.end()) {
			mDefendPositions[newDefendChokepoints[i]] = getDefendPosition(newDefendChokepoints[i]);
		}
	}


	// Remove choke points that doesn't need to be defended
	DefendMap::iterator oldDefendIt = mDefendPositions.begin();
	while (oldDefendIt != mDefendPositions.end()) {
		bool defendExists = false;
		size_t i = 0;
		while (i < newDefendChokepoints.size() && !defendExists) {
			if (oldDefendIt->first == newDefendChokepoints[i]) {
				defendExists = true;
			}
			++i;
		}

		// Did the existing defend pos in the new set
		if (!defendExists) {
			oldDefendIt = mDefendPositions.erase(oldDefendIt);
		} else {
			++oldDefendIt;
		}
	}
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
	const set<BWTA::Chokepoint*> emptyRegionChokepoints = pEmptyRegion->getChokepoints();
	vector<BWTA::Region*> emptyRegionNeigbors;

	// Find all neighbor regions to the empty region
	set<BWTA::Chokepoint*>::const_iterator chokePointIt;
	for (chokePointIt = emptyRegionChokepoints.begin(); chokePointIt != emptyRegionChokepoints.end(); ++chokePointIt) {
		// Don't check this choke point
		if (*chokePointIt != pChokepoint) {
			
			pair<BWTA::Region*, BWTA::Region*> borderRegions = (*chokePointIt)->getRegions();

			// One of the border regions is this region
			if (borderRegions.first != pEmptyRegion) {
				emptyRegionNeigbors.push_back(borderRegions.first);
			} else {
				emptyRegionNeigbors.push_back(borderRegions.second);
			}
		}
	}

	// Check if the any of the neighbors aren't occupied by use
	for (size_t i = 0; i < emptyRegionNeigbors.size(); ++i) {
		if (!isRegionOccupiedByOurTeam(emptyRegionNeigbors[i])) {
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

TilePosition DefenseManager::getCenterDefendPosition() const {
	TilePosition center(0,0);
	DefendMap::const_iterator defendPosIt;
	for (defendPosIt = mDefendPositions.begin(); defendPosIt != mDefendPositions.end(); ++defendPosIt) {
		center += defendPosIt->second.position;
	}

	if (!mDefendPositions.empty()) {
		center.x() /= mDefendPositions.size();
		center.y() /= mDefendPositions.size();
		return center;
	} else {
		return TilePositions::Invalid;
	}
}

TilePosition DefenseManager::getDefendPosition(BWTA::Chokepoint* pChokepoint) {
	const TilePosition chokePos(pChokepoint->getCenter());

	// Closest our closest structure. Used for calculating in which direction of the chokepoint
	// we shall be.
	const std::pair<Unit*, int>& closestStructure = getClosestAlliedStructure(chokePos);
	const TilePosition& structurePos = closestStructure.first->getTilePosition();

	int bestDist = INT_MAX;
	TilePosition defendPos = TilePositions::Invalid;
	int maxDist = config::defense::CHOKEPOINT_DISTANCE_MAX;

	int maxDistSquared = maxDist * maxDist;
	int minDistSquared = config::defense::CHOKEPOINT_DISTANCE_MIN * config::defense::CHOKEPOINT_DISTANCE_MIN;

	// Search for a good position to defend the choke point from
	for (int x = chokePos.x() - maxDist; x <= chokePos.x() + maxDist; x++) {
		for (int y = chokePos.y() - maxDist; y <= chokePos.y() + maxDist; y++) {
			TilePosition currentPos(x, y);

			if (ExplorationManager::canReach(currentPos, structurePos)) {
				int chokeDist = getSquaredDistance(chokePos, currentPos);

				if (chokeDist >= minDistSquared && chokeDist <= maxDistSquared) {
					int structureDist = getSquaredDistance(structurePos, currentPos);

					if (structureDist < bestDist) {
						bestDist = structureDist;
						defendPos = currentPos;
					}
				}
			}
		}
	}

	return defendPos;
}

void DefenseManager::updateMoveSquad() {
	// Get all free units, to be added to the defense move squads.
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


	// Get active move squad
	DefenseMoveSquadPtr activeMoveSquad;
	const vector<DefenseMoveSquadPtr>& moveSquads = mpSquadManager->getSquads<DefenseMoveSquad>();

	if (!moveSquads.empty()) {
		// We will always have only one DefenseMoveSquad (for now)
		activeMoveSquad = moveSquads[0];
	}


	// Add free units to the defense move squads, except transportations
	if (!freeUnits.empty()) {
		if (NULL == activeMoveSquad) {
			DefenseMoveSquad* pMoveSquad = new DefenseMoveSquad(freeUnits);
			activeMoveSquad = pMoveSquad->getThis();
			activeMoveSquad->activate();
		} else {
			activeMoveSquad->addUnits(freeUnits);
		}
	}


	// Update wait position and set defend position
	if (NULL != activeMoveSquad) {
		activeMoveSquad->setWaitPosition(getCenterDefendPosition());

		// Set a defend position if a position needs defending and the squad isn't defending an area
		if (!activeMoveSquad->isDefending() && mUnderAttack) {
			DefendMap::iterator defendIt = mDefendPositions.begin();
			while (defendIt != mDefendPositions.end() && !activeMoveSquad->isDefending()) {
				if (defendIt->second.underAttack) {
					activeMoveSquad->defendPosition(defendIt->second.position);
				}
				++defendIt;
			}
		}
	}
}

void DefenseManager::printGraphicDebugInfo() const {
	if (config::debug::GRAPHICS_VERBOSITY == config::debug::GraphicsVerbosity_Off ||
		config::debug::modules::DEFENSE == false)
	{
		return;
	}


	// Medium
	// Show defensive and offensive perimeter
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
		// Defensive & offensive
		DefendMap::const_iterator defendPosIt;
		for (defendPosIt = mDefendPositions.begin(); defendPosIt != mDefendPositions.end(); ++defendPosIt) {
			Position defendPos(defendPosIt->second.position);

			// Defend point
			Broodwar->drawDotMap(
				defendPos.x(),
				defendPos.y(),
				BWAPI::Colors::White
			);

			// Defend perimeter
			Broodwar->drawCircleMap(
				defendPos.x(),
				defendPos.y(),
				config::squad::defend::PERIMETER * TILE_SIZE,
				BWAPI::Colors::Brown
			);

			// Offensive perimeter
			Broodwar->drawCircleMap(
				defendPos.x(),
				defendPos.y(),
				config::squad::defend::ENEMY_OFFENSIVE_PERIMETER * TILE_SIZE,
				BWAPI::Colors::Red
			);
		}
	}

	// High
	// Show center of the defend positions
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_High) {
		// Center defend position
		Position centerPos(getCenterDefendPosition());

		Broodwar->drawCircleMap(
			centerPos.x(),
			centerPos.y(),
			2,
			BWAPI::Colors::Orange,
			true
		);
	}
}

void DefenseManager::updateUnderAttackPositions() {
	mUnderAttack = false;

	DefendMap::iterator defendPosIt;
	for (defendPosIt = mDefendPositions.begin(); defendPosIt != mDefendPositions.end(); ++defendPosIt) {
		if (isEnemyWithinRadius(defendPosIt->second.position, config::squad::defend::ENEMY_OFFENSIVE_PERIMETER)) {
			mUnderAttack = true;
			defendPosIt->second.underAttack = true;
		}
	}
}