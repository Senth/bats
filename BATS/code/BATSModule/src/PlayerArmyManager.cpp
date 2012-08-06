#include "PlayerArmyManager.h"
#include "Utilities/Logger.h"
#include "AlliedSquad.h"
#include "EnemySquad.h"
#include "Helper.h"
#include "Utilities/KeyHandler.h"
#include "BTHAIModule/Source/Profiler.h"
#include <cstdlib> // For NULL
#include <BWAPI/Game.h>
#include <BWAPI/Constants.h>
#include <BWAPI/Position.h>
#include <cassert>
#include <cmath>
#include <algorithm>

using namespace bats;
using namespace BWAPI;
using namespace std;
using std::tr1::shared_ptr;

PlayerArmyManager* PlayerArmyManager::mpsInstance = NULL;

const int GRID_RANGE_EXCLUDE_MAX = 2;
const int GRID_RANGE_EXCLUDE_CERTAIN = 1;

PlayerArmyManager::PlayerArmyManager() {
	recalculateLookupTable();

	config::addOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::GRID_SQUARE_DISTANCE), this);

	mLastFrameUpdate = 0;

	mSquads.resize(PlayerSquad::getMaxKeys());
}

PlayerArmyManager::~PlayerArmyManager() {
	config::removeOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::GRID_SQUARE_DISTANCE), this);
	mpsInstance = NULL;
}

PlayerArmyManager* PlayerArmyManager::getInstance() {
	if (NULL == mpsInstance) {
		mpsInstance = new PlayerArmyManager();
	}
	return mpsInstance;
}

void PlayerArmyManager::update() {
	

	// Computational heavy, don't call every frame.
	int cFrame = Broodwar->getFrameCount();
	if (cFrame - mLastFrameUpdate >= config::frame_distribution::ALLIED_ARMY_REARRANGE_SQUADS) {
		mLastFrameUpdate = cFrame;

		Profiler::getInstance()->start("AlliedArmyManager::update()");

		rearrangeSquads(true);
		rearrangeSquads(false);

		disbandEmptySquads();

		setAlliedFrontalSquad();

		Profiler::getInstance()->end("AlliedArmyManager::update()");
	}


	Profiler::getInstance()->start("PlayerSquads::update()");
	// Update all squads "every frame". Squads limit their update themselves
	for (size_t i = 0; i < mSquads.size(); ++i) {
		if (NULL != mSquads[i]) {
			mSquads[i]->update();
		}
	}
	Profiler::getInstance()->end("PlayerSquads::update()");
}

void PlayerArmyManager::rearrangeSquads(bool updateAllied) {
	Profiler::getInstance()->start("AlliedArmyManager::rearrangeSquads()");


	Profiler::getInstance()->start("AlliedArmyManager::rearrangeSquads() | Initialization");
		
	// Remove all existing units from the grid
	for (size_t x = 0; x < mGridUnits.size(); ++x) {
		for (size_t y = 0; y < mGridUnits[x].size(); ++y) {
			mGridUnits[x][y].clear();
		}
	}

	// Add all units to check and to the grid
	mUnitsToCheck.clear();
	std::map<BWAPI::Unit*, PlayerSquadId>::const_iterator unitIt;
	for (unitIt = mUnitSquad.begin(); unitIt != mUnitSquad.end(); ++unitIt) {
		if ((updateAllied && Broodwar->self()->isAlly(unitIt->first->getPlayer())) ||
			(!updateAllied && Broodwar->self()->isEnemy(unitIt->first->getPlayer())))
		{
			mUnitsToCheck[unitIt->first] = unitIt->second;
			addUnitToGrid(unitIt->first);
		}
	}

	Profiler::getInstance()->end("AlliedArmyManager::rearrangeSquads() | Initialization");
	Profiler::getInstance()->start("AlliedArmyManager::rearrangeSquads() | Find not checked unit");

	// Find a non-checked unit in a non-checked squad, i.e. iterate through all existing squads
	// first. This will avoid creating and then merging unnecessary squads. I.e. all units that
	// couldn't fit withing squads will then create new squads.
	std::set<PlayerSquadId> checkedSquads;

	bool foundUnit = false;
	// Iterate through all squads taking one unit each turn
	do {
		foundUnit = false;
		std::pair<BWAPI::Unit*, PlayerSquadId> currentUnit;
		currentUnit.first = NULL;

		// Is there a not checked unit with a not checked squad?
		std::map<BWAPI::Unit*, PlayerSquadId>::const_iterator unitIt = mUnitsToCheck.begin();
		while (unitIt != mUnitsToCheck.end() && !foundUnit) {
			if (unitIt->second != PlayerSquadId::INVALID_KEY &&
				checkedSquads.count(unitIt->second) == 0)
			{
				foundUnit = true;
				currentUnit = (*unitIt);

				unitIt = setUnitAsChecked(unitIt);
			} else {
				++unitIt;
			}
		}


		if (foundUnit) {
			// Set squad as checked
			checkedSquads.insert(currentUnit.second);

			addCloseUnitsToSquad(currentUnit.first, currentUnit.second);
		}

	} while (foundUnit);

	Profiler::getInstance()->end("AlliedArmyManager::rearrangeSquads() | Find not checked unit");
	Profiler::getInstance()->start("AlliedArmyManager::rearrangeSquads() | Create squads for rest");


	// Create squads for the rest of the units that went out of the squad's bounds
	while (!mUnitsToCheck.empty()) {
		// Create new squad
		PlayerSquadPtr pNewSquad;
		if (updateAllied) {
			pNewSquad = PlayerSquadPtr(new AlliedSquad());
		} else {
			pNewSquad = PlayerSquadPtr(new EnemySquad());
		}
		addSquad(pNewSquad);
		// Note: addCloseUnitsToSquad(), adds the unit to the squad automatically.

		BWAPI::Unit* currentUnit = mUnitsToCheck.begin()->first;
		setUnitAsChecked(mUnitsToCheck.begin());

		addCloseUnitsToSquad(currentUnit, pNewSquad->getId());
	}

	Profiler::getInstance()->end("AlliedArmyManager::rearrangeSquads() | Create squads for rest");
	Profiler::getInstance()->end("AlliedArmyManager::rearrangeSquads()");
}

void PlayerArmyManager::onConstantChanged(config::ConstantName constantName) {
	if (constantName == TO_CONSTANT_NAME(config::classification::squad::GRID_SQUARE_DISTANCE)) {
		recalculateLookupTable();
	}
}

void PlayerArmyManager::addUnit(BWAPI::Unit* pUnit) {
	// Note, this function does not update the squads, it simply add the unit to this class,
	// meaning in the next update phase the squads might be altered.
	
	// Don't add buildings and workers
	if (!pUnit->getType().isBuilding() && !pUnit->getType().isWorker()) {

		// This will create the unit with an INVALID_KEY for squad if it doesn't exist in the map
		// If it exist, it will do nothing.
		mUnitSquad[pUnit];
	}
}

void PlayerArmyManager::removeUnit(BWAPI::Unit* pUnit) {
	// Note, this function does not update the squads, it simply removes the unit from the squad
	// and this class, meaning in the next update phase the squads might be altered.
	std::map<Unit*, PlayerSquadId>::iterator unitIt = mUnitSquad.find(pUnit);
	if (unitIt != mUnitSquad.end() && unitIt->second.isValid()) {
		mSquads[unitIt->second]->removeUnit(pUnit);
		mUnitSquad.erase(unitIt);
	}
}

void PlayerArmyManager::addCloseUnitsToSquad(BWAPI::Unit* pUnit, PlayerSquadId squadId) {
	// Change squad if needed
	PlayerSquadId oldSquadId = mUnitSquad[pUnit];
	if (oldSquadId != squadId) {
		if (oldSquadId.isValid()) {
			mSquads[oldSquadId]->removeUnit(pUnit);
		}
		mSquads[squadId]->addUnit(pUnit);
		mUnitSquad[pUnit] = squadId;
	}

	
	// Indirectly recursive, first check all then recursive afterwards
	// Queue and checked units that are 100% certain in range of exclude range.
	// Includes same square, squares with direct borders to the current square,
	// not diagonally squares.
	std::list<Unit*> queuedUnits;

	//const TilePosition& unitPos = pUnit->getTilePosition();
	Position center = getGridPosition(pUnit);
	std::pair<Position, Position> gridRange = getValidGridRange(center, GRID_RANGE_EXCLUDE_CERTAIN);
	for (int x = gridRange.first.x(); x <= gridRange.second.x(); ++x) {
		for (int y = gridRange.first.y(); y <= gridRange.second.y(); ++y) {
			if (isSameOrBorderGridPosition(center, Position(x,y))) {
				
				// Iterate through all units within this square
				std::map<Unit*, bool>::iterator unitIt;
				for (unitIt = mGridUnits[x][y].begin(); unitIt != mGridUnits[x][y].end(); ++unitIt) {
					// Only parse not checked units
					if (unitIt->second == false) {
						// Only check same squads, these are 100% certain within exclude_distance
						// If the unit is part of the old (valid) squad, include those too
						PlayerSquadId unitSquadId = mUnitSquad[unitIt->first];
						if (unitSquadId == squadId || (oldSquadId.isValid() && unitSquadId == oldSquadId)) {
							queuedUnits.push_back(unitIt->first);
							setUnitAsChecked(unitIt->first);
						}
						// Another squad - check include range
						else if (withinIncludeDistance(pUnit, unitIt->first)) {
							queuedUnits.push_back(unitIt->first);
							setUnitAsChecked(unitIt->first);
						}
					}
				}
			}
		}
	}

	// Recursively checked queued units
	while (!queuedUnits.empty()) {
		addCloseUnitsToSquad(queuedUnits.front(), squadId);
		queuedUnits.pop_front();
	}


	// Directly recursive.
	// Check units out of 100% certain range of exclude range. Includes a grid of 5x5
	// squares, excluding those already checked. Includes units with different squads than
	// old squad and new squad.
	gridRange = getValidGridRange(center, GRID_RANGE_EXCLUDE_MAX);
	for (int x = gridRange.first.x(); x <= gridRange.second.x(); ++x) {
		for (int y = gridRange.first.y(); y <= gridRange.second.y(); ++y) {
			if (!isSameOrBorderGridPosition(center, Position(x,y))) {

				// Iterate through all units within this square
				std::map<Unit*, bool>::iterator unitIt;
				for (unitIt = mGridUnits[x][y].begin(); unitIt != mGridUnits[x][y].end(); ++unitIt) {
					// Only parse not checked units
					if (unitIt->second == false) {
						
						PlayerSquadId unitSquadId = mUnitSquad[unitIt->first];

						// Same squad or old (if valid), check exclude_distance
						if (unitSquadId == squadId || (oldSquadId.isValid() && unitSquadId == oldSquadId)) {
							if (withinExcludeDistance(pUnit, unitIt->first)) {
								setUnitAsChecked(unitIt->first);
								addCloseUnitsToSquad(unitIt->first, squadId);
							}
						}
						// Else - not same squad, check include_distance
						else {
							if (withinIncludeDistance(pUnit, unitIt->first)) {
								setUnitAsChecked(unitIt->first);
								addCloseUnitsToSquad(unitIt->first, squadId);
							}
						}
					}
				}
			}
		}
	}
}

void PlayerArmyManager::setUnitAsChecked(BWAPI::Unit* pUnit) {
	const Position& gridPos = getGridPosition(pUnit);
	if (gridPos != Positions::Invalid) {
		mGridUnits[gridPos.x()][gridPos.y()][pUnit] = true;
	}
	mUnitsToCheck.erase(pUnit);
}

std::map<BWAPI::Unit*, PlayerSquadId>::const_iterator PlayerArmyManager::setUnitAsChecked(const std::map<BWAPI::Unit*, PlayerSquadId>::const_iterator& unitIt) {
	const Position& gridPos = getGridPosition(unitIt->first);
	if (gridPos != Positions::Invalid) {
		mGridUnits[gridPos.x()][gridPos.y()][unitIt->first] = true;
	}
	return mUnitsToCheck.erase(unitIt);
}

bool PlayerArmyManager::withinExcludeDistance(BWAPI::Unit* pUnitA, BWAPI::Unit* pUnitB) const {
	return getSquaredDistance(pUnitA->getTilePosition(), pUnitB->getTilePosition()) <=
		config::classification::squad::EXCLUDE_DISTANCE_SQUARED;
}

bool PlayerArmyManager::withinIncludeDistance(BWAPI::Unit* pUnitA, BWAPI::Unit* pUnitB) const {
	return getSquaredDistance(pUnitA->getTilePosition(), pUnitB->getTilePosition()) <=
		config::classification::squad::INCLUDE_DISTANCE_SQUARED;
}

BWAPI::Position PlayerArmyManager::getGridPosition(BWAPI::Unit* pUnit) const {
	TilePosition unitPos = BWAPI::TilePositions::Invalid;

	// Loaded
	if (pUnit->isLoaded()) {
		// Get position of the transport, this is needed because the position of the units
		// are not updated while loaded.
		Unit* pTransport = pUnit->getTransport();
		assert(NULL != pTransport);
		if (NULL != pTransport) {
			unitPos = pTransport->getTilePosition();
		} else {
			ERROR_MESSAGE(false, "Unit is loaded, but doesn't have a transport!");
			unitPos = pUnit->getTilePosition();
		}
	}
	// Use regular position
	else {
		unitPos = pUnit->getTilePosition();
	}

	Position gridPos = Positions::Invalid;
	if (unitPos.isValid()) {
		gridPos = mLookupTableGridPosition[unitPos.x()][unitPos.y()];
	} else if (unitPos != TilePositions::Invalid && unitPos != TilePositions::None && unitPos != TilePositions::Unknown) {
		ERROR_MESSAGE(false, "Invalid unit " << pUnit->getType().getName() <<
			" (" << pUnit->getID() << ") position: " << unitPos);
	}

	return gridPos;
}

bool PlayerArmyManager::isSameOrBorderGridPosition(const BWAPI::Position& centerPosition, const BWAPI::Position& checkPosition) const {
	// Only same or border if maximum one coordinate is from the center,
	// i.e. 0,0 center, 0,1, 1,0, -1,0, 0,-1 returns true
	int xDiff = abs(centerPosition.x() - checkPosition.x());
	int yDiff = abs(centerPosition.y() - checkPosition.y());

	return ((xDiff <= 1 && yDiff == 0) || (yDiff <= 1 && xDiff == 0));
}

std::pair<Position, Position> PlayerArmyManager::getValidGridRange(const BWAPI::Position centerPosition, int range) const {
	Position min(centerPosition.x() - range, centerPosition.y() - range);
	if (min.x() < 0) {
		min.x() = 0;
	}
	if (min.y() < 0) {
		min.y() = 0;
	}

	Position max(centerPosition.x() + range, centerPosition.y() + range);
	if (max.x() >= static_cast<int>(mGridUnits.size())) {
		max.x() = static_cast<int>(mGridUnits.size() - 1);
	}
	if (max.y() >= static_cast<int>(mGridUnits[0].size())) {
		max.y() = static_cast<int>(mGridUnits[0].size() - 1);
	}

	return std::make_pair(min, max);
}

void PlayerArmyManager::recalculateLookupTable() {
	int gridSquareDistance = config::classification::squad::GRID_SQUARE_DISTANCE;

	mLookupTableGridPosition.clear();
	mLookupTableGridPosition.resize(BWAPI::Broodwar->mapWidth());

	for (int x = 0; x < BWAPI::Broodwar->mapWidth(); x++) {
		mLookupTableGridPosition[x].resize(BWAPI::Broodwar->mapHeight());
		for (int y = 0; y < BWAPI::Broodwar->mapHeight(); y++) {
			mLookupTableGridPosition[x][y] = BWAPI::Position(x / gridSquareDistance, y / gridSquareDistance);
		}
	}

	// Initialize size of grid
	mGridUnits.clear();
	mGridUnits.resize(mLookupTableGridPosition.size());
	for (size_t x = 0; x < mGridUnits.size(); ++x) {
		mGridUnits[x].resize(mLookupTableGridPosition[0].size());
	}
}

void PlayerArmyManager::disbandEmptySquads() {
	for (size_t i = 0; i < mSquads.size(); ++i) {
		if (NULL != mSquads[i]) {
			if (mSquads[i]->isEmpty()) {
				mSquads[i].reset();
			}
		}
	}
}

void PlayerArmyManager::setAlliedFrontalSquad() {
	// Save biggest squad and set all squads as small
	AlliedSquadPtr biggestSquad;
	int cBiggestSize = 0;
	for (size_t i = 0; i < mSquads.size(); ++i) {
		AlliedSquadPtr pAlliedSquad = std::tr1::dynamic_pointer_cast<AlliedSquad>(mSquads[i]);
		if (NULL != pAlliedSquad) {
			pAlliedSquad->setFrontalAttack(false);

			int currentSupplyCount = pAlliedSquad->getSupplyCount();
			if (currentSupplyCount > cBiggestSize) {
				biggestSquad = pAlliedSquad;
				cBiggestSize = currentSupplyCount;
			}
		}
	}


	if (NULL != biggestSquad) {
		biggestSquad->setFrontalAttack(true);
	}
}

void PlayerArmyManager::addSquad(PlayerSquadPtr pSquad) {
	mSquads[pSquad->getId()] = pSquad;
}

void PlayerArmyManager::removeSquad(PlayerSquadId squadId) {
	mSquads[squadId].reset();
}

void PlayerArmyManager::addUnitToGrid(BWAPI::Unit* pUnit) {
	Position gridPos = getGridPosition(pUnit);

	if (gridPos != Positions::Invalid) {
		mGridUnits[gridPos.x()][gridPos.y()][pUnit] = false;
	}
}

AlliedSquadCstPtr PlayerArmyManager::getAlliedFrontalSquad() const {
	for (size_t i = 0; i < mSquads.size(); ++i) {
		AlliedSquadPtr pAlliedSquad = std::tr1::dynamic_pointer_cast<AlliedSquad>(mSquads[i]);
		if (NULL != pAlliedSquad && pAlliedSquad->isFrontalAttack()) {
			return pAlliedSquad;
		}
	}
	return AlliedSquadCstPtr();
}



void PlayerArmyManager::printGraphicDebugInfo() const {
	// Skip if not turned on
	if (config::debug::GRAPHICS_VERBOSITY == config::debug::GraphicsVerbosity_Off) {
		return;
	}


	// Only draw if turned on
	if (config::debug::modules::ALLIED_ARMY_MANAGER) {
		// High 
		// Draw two circles around each unit, orange for excluding, green for including
		// Draw id on each unit
		if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_High) {
			// Convert to map coordinates
			int includeDistance = config::classification::squad::INCLUDE_DISTANCE * TILE_SIZE >> 1;
			int excludeDistance = config::classification::squad::EXCLUDE_DISTANCE * TILE_SIZE >> 1;

			std::map<Unit*, PlayerSquadId>::const_iterator unitIt;
			for (unitIt = mUnitSquad.begin(); unitIt != mUnitSquad.end(); ++unitIt) {
				// Include distance
				Broodwar->drawCircleMap(
					unitIt->first->getPosition().x(),
					unitIt->first->getPosition().y(),
					includeDistance,
					Colors::Green
				);

				// Exclude distance
				Broodwar->drawCircleMap(
					unitIt->first->getPosition().x(),
					unitIt->first->getPosition().y(),
					excludeDistance,
					Colors::Orange
				);

				// Id on unit
				Broodwar->drawTextMap(
					unitIt->first->getPosition().x(),
					unitIt->first->getPosition().y(),
					"\x04%i",
					unitIt->first->getID()
				);
			}
		}
	}


	// Print all squads
	for (size_t i = 0; i < mSquads.size(); ++i) {
		if (NULL != mSquads[i]) {
			mSquads[i]->printGraphicDebugInfo();
		}
	}	
}