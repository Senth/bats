#include "DefenseManager.h"
#include "UnitManager.h"
#include "SquadManager.h"
#include "PatrolSquad.h"
#include "HoldSquad.h"
#include "Helper.h"
#include "Config.h"
#include "ExplorationManager.h"
#include "GameTime.h"
#include "TypeDefs.h"
#include "Utilities/Logger.h"
#include "UnitCompositionFactory.h"
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
	mpUnitCompositionFactory = NULL;

	mFrameCallLast = 0;
	mpUnitManager = UnitManager::getInstance();
	mpSquadManager = SquadManager::getInstance();
	mpGameTime = GameTime::getInstance();
	mpUnitCompositionFactory = UnitCompositionFactory::getInstance();
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
	updatePatrolSquad();
	updateHoldSquads();
}

void DefenseManager::updateHoldSquads() {
	vector<HoldSquadPtr> holdSquads = mpSquadManager->getSquads<HoldSquad>();

	// Remove any DefenseHoldSquad in position that shall not be defended any longer.
	vector<HoldSquadPtr>::iterator holdIt = holdSquads.begin();
	while (holdIt != holdSquads.end()) {
		if (!isInDefendingList((*holdIt)->getDefendPosition())) {
			(*holdIt)->tryDisband();
			DEBUG_MESSAGE(utilities::LogLevel_Finer,
				"DefenseManager: Removing hold squad from undefended area. Unit composition: " <<
				(*holdIt)->getUnitComposition().getName() << ", defend: " <<
				(*holdIt)->getDefendPosition() << ", roam: " << (*holdIt)->getRoamPosition()
			);
			holdIt = holdSquads.erase(holdIt);
		} else {
			++holdIt;
		}
	}


	// Need free units from the patrol squad.
	vector<PatrolSquadPtr> patrolSquads = mpSquadManager->getSquads<PatrolSquad>();
	if (!patrolSquads.empty()) {
		PatrolSquadPtr pPatrolSquad = patrolSquads.front();


		// Is any defense point undefended, create DefenseHoldSquad for that position
		// @todo Prioritize bot's defense points.
		DefendSet::const_iterator defendIt;
		for (defendIt = mDefendPositions.begin(); defendIt != mDefendPositions.end(); ++defendIt) {
			bool defended = false;
			size_t squadIndex = 0;
			while (!defended && squadIndex < holdSquads.size()) {
				if (holdSquads[squadIndex]->getDefendPosition() == defendIt->position) {
					defended = true;
				}
				++squadIndex;
			}

			if (!defended) {
				// Copy units, else the vector will be corrupt when adding units
				const vector<UnitAgent*> patrolUnits = pPatrolSquad->getUnits();
				const vector<UnitComposition>& unitCompositions =
					mpUnitCompositionFactory->getUnitCompositionsByType(patrolUnits, UnitComposition_Defend);

				// Create a new defense squad
				if (!unitCompositions.empty()) {
					const TilePosition& roamPosition = findRoamPosition(defendIt->position);
					new HoldSquad(patrolUnits, unitCompositions.front(), defendIt->position, roamPosition);
					DEBUG_MESSAGE(utilities::LogLevel_Finer,
						"DefenseManager: Found an undefended area, created a new Hold Squad, " <<
						"Composition: " << unitCompositions.front().getName() << ", defend: " <<
						defendIt->position << ", roam: " << roamPosition
					);
				}
			}
		}
	

		// Create higher priority composition for any defended area, if possible
		holdIt = holdSquads.begin();
		while (holdIt != holdSquads.end()) {
			vector<UnitAgent*> units = pPatrolSquad->getUnits();
			// Add units from the existing hold squad too.
			units.insert(units.end(), (*holdIt)->getUnits().begin(), (*holdIt)->getUnits().end());
			const vector<UnitComposition>& unitCompositions =
				mpUnitCompositionFactory->getUnitCompositionsByType(units, UnitComposition_Defend);

			if (!unitCompositions.empty() &&
				unitCompositions.front().getPriority() > (*holdIt)->getUnitComposition().getPriority())
			{
				// Create new roam position, maybe a better one exist now?
				const BWAPI::TilePosition& roamPosition = findRoamPosition((*holdIt)->getDefendPosition());
				new HoldSquad(units, unitCompositions.front(), (*holdIt)->getDefendPosition(), roamPosition);
				DEBUG_MESSAGE(utilities::LogLevel_Finer,
					"DefenseManager: Upgraded hold squad, from composition '" <<
					(*holdIt)->getUnitComposition().getName() << "' to '" <<
					unitCompositions.front().getName() << "' in defend: " <<
					(*holdIt)->getDefendPosition()
				);

				// Disband and delete squad
				(*holdIt)->tryDisband();
				holdIt = holdSquads.erase(holdIt);
			} else {
				++holdIt;
			}
		}


		// Add more units to not full Hold Squads
		for (size_t i = 0; i < holdSquads.size(); ++i) {
			if (!holdSquads[i]->isFull()) {
				// Copy units else the vector will be corrupt when adding units
				vector<UnitAgent*> patrolUnits = pPatrolSquad->getUnits();
				holdSquads[i]->addUnits(patrolUnits);
			}
			
		}
	}
}

vector<UnitAgent*> DefenseManager::getFreeUnits() {
	// Never return units when under attack
	if (isUnderAttack()) {
		return vector<UnitAgent*>();
	}


	vector<UnitAgent*> freeUnits;

	// Get units from the patrol squad
	vector<PatrolSquadPtr> patrolSquads = mpSquadManager->getSquads<PatrolSquad>();
	for (size_t i = 0; i < patrolSquads.size(); ++i) {
		const vector<UnitAgent*>& squadUnits = patrolSquads[i]->getUnits();
		freeUnits.insert(freeUnits.end(), squadUnits.begin(), squadUnits.end());
	}

	// Get units from hold squads
	vector<HoldSquadPtr> holdSquads = mpSquadManager->getSquads<HoldSquad>();
	for (size_t i = 0; i < holdSquads.size(); ++i) {
		const vector<UnitAgent*>& squadUnits = holdSquads[i]->getUnits();
		freeUnits.insert(freeUnits.end(), squadUnits.begin(), squadUnits.end());
	}

	return freeUnits;
}

bool DefenseManager::isUnderAttack() const {
	return mUnderAttack;
}

void DefenseManager::updateDefendPositions() {
	vector<BWTA::Chokepoint*> newDefendChokepoints = getDefendChokepoints();
	
	// Set defend positions for all new defend choke points
	for (size_t i = 0; i < newDefendChokepoints.size(); ++i) {
		DefendSet::const_iterator foundIt = mDefendPositions.find(newDefendChokepoints[i]);

		if (foundIt == mDefendPositions.end()) {
			DefendPosition newDefendPosition(newDefendChokepoints[i]);

			// Check if the defend position belongs to us
			if (isOurOrAlliedChokepoint(newDefendChokepoints[i], true)) {
				newDefendPosition.isOur = true;
			} else {
				newDefendPosition.isOur = false;
			}

			// Check if defend position belongs to allied
			if (isOurOrAlliedChokepoint(newDefendChokepoints[i], false)) {
				newDefendPosition.isAllied = true;
			} else {
				newDefendPosition.isAllied = false;
			}

			mDefendPositions.insert(newDefendPosition);
		}
	}


	// Remove choke points that doesn't need to be defended
	DefendSet::iterator oldDefendIt = mDefendPositions.begin();
	while (oldDefendIt != mDefendPositions.end()) {
		bool defendExists = false;
		size_t i = 0;
		while (i < newDefendChokepoints.size() && !defendExists) {
			if (oldDefendIt->pChokepoint == newDefendChokepoints[i]) {
				defendExists = true;
			}
			++i;
		}

		// Didn't find the existing position in the new set of positions -> remove it.
		if (!defendExists) {
			oldDefendIt = mDefendPositions.erase(oldDefendIt);
		} else {
			++oldDefendIt;
		}
	}
}

bool DefenseManager::isRegionOccupiedByOurTeam(BWTA::Region* pRegion, bool includeOurStructures, bool includeAlliedStructures) {
	assert(NULL != pRegion);
	if (NULL == pRegion) {
		return false;
	}

	set<Player*> teamPlayers;
	if (includeAlliedStructures) {
		teamPlayers = Broodwar->allies();
	}
	if (includeOurStructures) {
		teamPlayers.insert(Broodwar->self());
	}

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
	assert(NULL != pChokepoint);

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

TilePosition DefenseManager::findRoamPosition(const BWAPI::TilePosition& defendPosition) {

	// Closest our closest structure. Used for calculating in which direction of the choke point
	// we shall be.
	const std::pair<Unit*, int>& closestStructure = getClosestAlliedStructure(defendPosition);
	const TilePosition& structurePos = closestStructure.first->getTilePosition();

	int bestDist = INT_MAX;
	TilePosition roamPosition = TilePositions::Invalid;
	int maxDist = config::squad::defend::ROAM_DISTANCE_MAX;

	int maxDistSquared = maxDist * maxDist;
	int minDistSquared = config::squad::defend::ROAM_DISTANCE_MIN * config::squad::defend::ROAM_DISTANCE_MIN;

	// Search for a good position to defend the choke point from
	for (int x = defendPosition.x() - maxDist; x <= defendPosition.x() + maxDist; x++) {
		for (int y = defendPosition.y() - maxDist; y <= defendPosition.y() + maxDist; y++) {
			TilePosition currentPos(x, y);

			/// @todo make sure it's the right region
			if (ExplorationManager::canReach(currentPos, structurePos)) {
				int roamDistance = getSquaredDistance(defendPosition, currentPos);

				if (roamDistance >= minDistSquared && roamDistance <= maxDistSquared) {
					int structureDist = getSquaredDistance(structurePos, currentPos);

					if (structureDist < bestDist) {
						bestDist = structureDist;
						roamPosition = currentPos;
					}
				}
			}
		}
	}

	return roamPosition;
}

void DefenseManager::updatePatrolSquad() {
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
	PatrolSquadPtr activePatrolSquad;
	const vector<PatrolSquadPtr>& moveSquads = mpSquadManager->getSquads<PatrolSquad>();

	if (!moveSquads.empty()) {
		// We will always have only one DefenseMoveSquad (for now)
		activePatrolSquad = moveSquads[0];
	}


	// Add free units to the defense move squads, except transportations
	if (!freeUnits.empty()) {
		if (NULL == activePatrolSquad) {
			PatrolSquad* pMoveSquad = new PatrolSquad(freeUnits);
			activePatrolSquad = pMoveSquad->getThis();
			activePatrolSquad->activate();
		} else {
			activePatrolSquad->addUnits(freeUnits);
		}
	}


	// Set patrol position and defend position
	if (NULL != activePatrolSquad) {
		// Patrol
		// Get all our defended positions
		set<TilePosition> patrolPositions;
		for (DefendSet::const_iterator defendPosIt = mDefendPositions.begin(); defendPosIt != mDefendPositions.end(); ++defendPosIt) {
			if (defendPosIt->isOur) {
				patrolPositions.insert(defendPosIt->position);
			}
		}
		activePatrolSquad->setPatrolPositions(patrolPositions);

		// Defend - if a position needs defending and the squad isn't defending an area
		if (!activePatrolSquad->isDefending() && mUnderAttack) {
			DefendSet::iterator defendIt = mDefendPositions.begin();
			while (defendIt != mDefendPositions.end() && !activePatrolSquad->isDefending()) {
				if (defendIt->underAttack) {
					activePatrolSquad->defendPosition(defendIt->position);
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
	// Show roaming, defensive and offensive perimeter
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
		// Defensive & offensive
		DefendSet::const_iterator defendPosIt;
		for (defendPosIt = mDefendPositions.begin(); defendPosIt != mDefendPositions.end(); ++defendPosIt) {
			Position defendPos(defendPosIt->position);

			// Defend perimeter
			Broodwar->drawCircleMap(
				defendPos.x(),
				defendPos.y(),
				config::squad::defend::DEFEND_PERIMETER * TILE_SIZE,
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
}

void DefenseManager::updateUnderAttackPositions() {
	mUnderAttack = false;

	DefendSet::iterator defendPosIt;
	for (defendPosIt = mDefendPositions.begin(); defendPosIt != mDefendPositions.end(); ++defendPosIt) {
		if (isEnemyWithinRadius(defendPosIt->position, config::squad::defend::ENEMY_OFFENSIVE_PERIMETER)) {
			mUnderAttack = true;
			defendPosIt->underAttack = true;
		} else {
			defendPosIt->underAttack = false;
		}
	}
}

bool DefenseManager::isOurOrAlliedChokepoint(BWTA::Chokepoint* pChokepoint, bool testOur) {
	assert(NULL != pChokepoint);

	const pair<BWTA::Region*, BWTA::Region*> regions = pChokepoint->getRegions();

	if (isRegionOccupiedByOurTeam(regions.first, testOur, !testOur)) {
		return true;
	} else if (isRegionOccupiedByOurTeam(regions.second, testOur, !testOur)) {
		return true;
	} else {
		return false;
	}
}

bool DefenseManager::isInDefendingList(const BWAPI::TilePosition& position) const {
	DefendSet::const_iterator it;
	for (it = mDefendPositions.begin(); it != mDefendPositions.end(); ++it) {
		if (it->position == position) {
			return true;
		}
	}

	return false;
}