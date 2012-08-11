#include "DefenseManager.h"
#include "UnitManager.h"
#include "IntentionWriter.h"
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

DefenseManager* DefenseManager::msInstance = NULL;

DefenseManager::DefenseManager() {
	mUnitManager = NULL;
	mSquadManager = NULL;
	mGameTime = NULL;
	mUnitCompositionFactory = NULL;
	mDefendUnit = NULL;
	mDefendAlliedUnit = NULL;
	mIntentionWriter = NULL;

	mUnitManager = UnitManager::getInstance();
	mSquadManager = SquadManager::getInstance();
	mGameTime = GameTime::getInstance();
	mUnitCompositionFactory = UnitCompositionFactory::getInstance();
	mIntentionWriter = IntentionWriter::getInstance();

	mUnderAttack = false;
	mAlliedUnderAttack = false;
	mDefendingAllied = false;

	mFrameCallLast = 0;
}

DefenseManager::~DefenseManager() {
	msInstance = NULL;
}

DefenseManager* DefenseManager::getInstance() {
	if (NULL == msInstance) {
		msInstance = new DefenseManager();
	}
	return msInstance;
}

void DefenseManager::update() {
	// Don't update too often
	if (mFrameCallLast + config::frame_distribution::DEFENSE_MANAGER > mGameTime->getFrameCount()) {
		return;
	}
	mFrameCallLast = mGameTime->getFrameCount();

	updateDefendPositions();
	updateUnderAttackPositions();
	updatePatrolSquad();
	updateHoldSquads();
}

void DefenseManager::updateHoldSquads() {
	vector<HoldSquadPtr> holdSquads = mSquadManager->getSquads<HoldSquad>();

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
	vector<PatrolSquadPtr> patrolSquads = mSquadManager->getSquads<PatrolSquad>();
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
					mUnitCompositionFactory->getUnitCompositionsByType(patrolUnits, UnitComposition_Defend);

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
				mUnitCompositionFactory->getUnitCompositionsByType(units, UnitComposition_Defend);

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
	vector<PatrolSquadPtr> patrolSquads = mSquadManager->getSquads<PatrolSquad>();
	for (size_t i = 0; i < patrolSquads.size(); ++i) {
		const vector<UnitAgent*>& squadUnits = patrolSquads[i]->getUnits();
		freeUnits.insert(freeUnits.end(), squadUnits.begin(), squadUnits.end());
	}

	// Get units from hold squads
	vector<HoldSquadPtr> holdSquads = mSquadManager->getSquads<HoldSquad>();
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

bool DefenseManager::isRegionOccupiedByOurTeam(BWTA::Region* region, bool includeOurStructures, bool includeAlliedStructures) {
	assert(NULL != region);
	if (NULL == region) {
		return false;
	}

	set<Player*> teamPlayers;
	if (includeAlliedStructures) {
		teamPlayers = Broodwar->allies();
	}
	if (includeOurStructures) {
		teamPlayers.insert(Broodwar->self());
	}

	const Position regionToCheckPos = region->getCenter();

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

bool DefenseManager::isChokepointEdge(BWTA::Chokepoint* chokepoint) {
	assert(NULL != chokepoint);

	const pair<BWTA::Region*, BWTA::Region*>& abutRegions = chokepoint->getRegions();
	BWTA::Region* occupiedRegion = NULL;
	BWTA::Region* emptyRegion = NULL;

	if (isRegionOccupiedByOurTeam(abutRegions.first)) {
		occupiedRegion = abutRegions.first;
	} else {
		emptyRegion = abutRegions.first;
	}

	// Now the second region needs to be inverse of the first region.
	// E.g. first -> occupied, second -> empty.
	if (isRegionOccupiedByOurTeam(abutRegions.second)) {
		if (NULL == occupiedRegion) {
			occupiedRegion = abutRegions.second;
		} else {
			return false;
		}
	} else {
		if (NULL == emptyRegion) {
			emptyRegion = abutRegions.second;
		} else {
			return false;
		}
	}
	

	// The empty region needs to abut another empty region.
	const set<BWTA::Chokepoint*> emptyRegionChokepoints = emptyRegion->getChokepoints();
	vector<BWTA::Region*> emptyRegionNeigbors;

	// Find all neighbor regions to the empty region
	set<BWTA::Chokepoint*>::const_iterator chokePointIt;
	for (chokePointIt = emptyRegionChokepoints.begin(); chokePointIt != emptyRegionChokepoints.end(); ++chokePointIt) {
		// Don't check this choke point
		if (*chokePointIt != chokepoint) {
			
			pair<BWTA::Region*, BWTA::Region*> borderRegions = (*chokePointIt)->getRegions();

			// One of the border regions is this region
			if (borderRegions.first != emptyRegion) {
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
	vector<UnitAgent*> freeUnits = mUnitManager->getUnitsByFilter(UnitFilter_HasNoSquad);

	// Remove transportations
	vector<UnitAgent*>::const_iterator unitIt = freeUnits.begin();
	while (unitIt != freeUnits.end()) {
		if ((*unitIt)->isTransport()) {
			unitIt = freeUnits.erase(unitIt);
		} else {
			++unitIt;
		}
	}


	PatrolSquadPtr activePatrolSquad = getPatrolSquad();

	// Add remaining units
	if (!freeUnits.empty()) {
		if (NULL == activePatrolSquad) {
			PatrolSquad* pNewSquad = new PatrolSquad(freeUnits);
			activePatrolSquad = pNewSquad->getThis();
			activePatrolSquad->activate();
		} else {
			activePatrolSquad->addUnits(freeUnits);
		}
	}


	// Set patrol position and defend position
	if (NULL != activePatrolSquad) {
		// If patrol squad is not defending anything, that means we're not defending an allied either
		if (!activePatrolSquad->isDefending()) {
			mDefendingAllied = false;
		}

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

					// Defending player? Tell him that
					if (defendIt->isAllied && !defendIt->isOur) {
						mIntentionWriter->writeIntention(Intention_BotComingToAid);
						mDefendingAllied = true;
					}
				}
				++defendIt;
			}
		}

		// Still not defending anything, check if allied need defending help with a structure?
		if (!activePatrolSquad->isDefending() && NULL != mDefendAlliedUnit) {
			mDefendingAllied = true;
			mIntentionWriter->writeIntention(Intention_BotComingToAid);
			defendUnit(mDefendAlliedUnit);
		}

		if (!mDefendingAllied && mAlliedUnderAttack) {
			mIntentionWriter->writeIntention(Intention_BotComingToAidNot, Reason_BotIsUnderAttack);
		}
	} else {
		mDefendingAllied = false;

		if (mAlliedUnderAttack) {
			mIntentionWriter->writeIntention(Intention_BotComingToAidNot, Reason_BotNotEnoughUnits);
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
	mAlliedUnderAttack = false;

	// Defend unit
	if (NULL != mDefendUnit) {
		if (mDefendUnit->isUnderAttack()) {
			mUnderAttack = true;
		} else {
			mDefendUnit = NULL;
		}
	}
	// Allied defend unit
	if (NULL != mDefendAlliedUnit) {
		if (mDefendAlliedUnit->isUnderAttack()) {
			mUnderAttack = true;
			mAlliedUnderAttack = true;
		} else {
			mDefendAlliedUnit = NULL;
		}
	}


	// Hold choke points
	DefendSet::iterator defendPosIt;
	for (defendPosIt = mDefendPositions.begin(); defendPosIt != mDefendPositions.end(); ++defendPosIt) {
		if (isEnemyWithinRadius(defendPosIt->position, config::squad::defend::ENEMY_OFFENSIVE_PERIMETER)) {
			mUnderAttack = true;
			defendPosIt->underAttack = true;

			if (defendPosIt->isAllied && !defendPosIt->isOur) {
				mAlliedUnderAttack = true;
			}

		} else {
			defendPosIt->underAttack = false;
		}
	}


	// If patrol squad is defending somewhere, we're under attack
	PatrolSquadPtr patrolSquad = getPatrolSquad();
	if (NULL != patrolSquad && patrolSquad->isDefending()) {
		mUnderAttack = true;
	}


	// Check if allied is under attack, break when is under attack
	if (!mAlliedUnderAttack) {
		const set<Player*>& allies = Broodwar->allies();
		set<Player*>::const_iterator alliedIt = allies.begin();
		while (!mAlliedUnderAttack && alliedIt != allies.end()) {
			const set<Unit*>& units = (*alliedIt)->getUnits();
			set<Unit*>::const_iterator unitIt = units.begin();
			while (!mAlliedUnderAttack && unitIt != units.end()) {
				if ((*unitIt)->getType().isBuilding() && (*unitIt)->isUnderAttack()) {
					mAlliedUnderAttack = true;
					mDefendAlliedUnit = (*unitIt);
				}
			}
		}
	}
}

bool DefenseManager::isOurOrAlliedChokepoint(BWTA::Chokepoint* chokepoint, bool testOur) {
	assert(NULL != chokepoint);

	const pair<BWTA::Region*, BWTA::Region*> regions = chokepoint->getRegions();

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

TilePosition DefenseManager::findRetreatPosition() const {
	TilePosition retreatPos = TilePositions::Invalid;

	// Search for our defend positions first
	set<DefendPosition>::const_iterator defendIt = mDefendPositions.begin();
	while (retreatPos == TilePositions::Invalid && defendIt != mDefendPositions.end()) {
		if (defendIt->isOur) {
			retreatPos = defendIt->position;
		}
		++defendIt;
	}
	
	// Did not find any defend position, use our start location
	if (retreatPos == TilePositions::Invalid) {
		retreatPos = Broodwar->self()->getStartLocation();
	}

	return retreatPos;
}

void DefenseManager::onStructureUnderAttack(BaseAgent* structure) {
	defendUnit(structure->getUnit());
}

#pragma warning(push)
#pragma warning(disable:4100)
void DefenseManager::onWorkerUnderAttack(BaseAgent* worker) {
	defendUnit(worker->getUnit());

	/// @todo Make the worker and close by workers retreat to a location in the meanwhile.
}
#pragma warning(pop)

void DefenseManager::defendUnit(Unit* unit) {
	if (NULL == mDefendUnit && !isUnderAttack()) {
		PatrolSquadPtr patrolSquad = getPatrolSquad();

		if (NULL != patrolSquad && !patrolSquad->isDefending()) {
			mDefendUnit = unit;
			mUnderAttack = true;
			patrolSquad->defendPosition(mDefendUnit->getTilePosition(), true);
		}
	}
}

PatrolSquadPtr DefenseManager::getPatrolSquad() {
	PatrolSquadPtr activePatrolSquad;
	const vector<PatrolSquadPtr>& moveSquads = mSquadManager->getSquads<PatrolSquad>();

	if (!moveSquads.empty()) {
		// We will always have only one DefenseMoveSquad (for now)
		activePatrolSquad = moveSquads[0];
	}

	return activePatrolSquad;
}

PatrolSquadCstPtr DefenseManager::getPatrolSquad() const {
	return const_cast<DefenseManager*>(this)->getPatrolSquad();
}