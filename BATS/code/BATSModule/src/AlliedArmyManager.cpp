#include "AlliedArmyManager.h"
#include "Utilities/Logger.h"
#include "AlliedSquad.h"
#include "Helper.h"
#include <cstdlib> // For NULL
#include <BWAPI/Game.h>
#include <cassert>
#include <cmath>

using namespace bats;
using namespace BWAPI;
using std::tr1::shared_ptr;

AlliedArmyManager* AlliedArmyManager::mpsInstance = NULL;

const int GRID_RANGE_EXCLUDE_MAX = 2;
const int GRID_RANGE_EXCLUDE_CERTAIN = 1;

AlliedArmyManager::AlliedArmyManager() {
	recalculateLookupTable();

	config::addOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::GRID_SQUARE_DISTANCE), this);

	mLastFrameUpdate = 0;

	// Initialize size of grid
	mGridUnits.resize(mLookupTableGridPosition.size());
	for (size_t x = 0; x < mGridUnits.size(); ++x) {
		mGridUnits[x].resize(mLookupTableGridPosition[0].size());
	}
}

AlliedArmyManager::~AlliedArmyManager() {
	config::removeOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::GRID_SQUARE_DISTANCE), this);
	mpsInstance = NULL;
}

AlliedArmyManager* AlliedArmyManager::getInstance() {
	if (NULL == mpsInstance) {
		mpsInstance = new AlliedArmyManager();
	}
	return mpsInstance;
}

void AlliedArmyManager::update() {
	// Computational heavy, don't call every frame.
	int cFrame = Broodwar->getFrameCount();
	if (cFrame - mLastFrameUpdate >= config::frame_distribution::ALLIED_ARMY_REARRANGE_SQUADS) {
		rearrangeSquads();

		disbandEmptySquads();

		setBigSquad();
	}


	// Update all squads "every frame". Squads limit their update themselves
	for (size_t i = 0; i < mSquads.size(); ++i) {
		if (NULL != mSquads[i]) {
			mSquads[i]->update();
		}
	}
}

void AlliedArmyManager::rearrangeSquads() {
	mUnitsToCheck = mUnitSquad;


	// Create the grid with units
	// 2D grid, each square can hold any number of units (3rd vector), each unit has information
	// if they have been checked or not (bool in pair).
	mGridUnits.clear();

	// Initialize size of grid
	mGridUnits.resize(mLookupTableGridPosition.size());
	for (size_t x = 0; x < mGridUnits.size(); ++x) {
		mGridUnits[x].resize(mLookupTableGridPosition[0].size());
	}

	// Add all units to the grid
	std::map<BWAPI::Unit*, AlliedSquadId>::const_iterator unitIt;
	for (unitIt = mUnitsToCheck.begin(); unitIt != mUnitsToCheck.end(); ++unitIt) {
		addUnitToGrid(unitIt->first);
	}


	// Find a non-checked unit in a non-checked squad, i.e. iterate through all existing squads
	// first. This will avoid creating and then merging unnecessary squads. I.e. all units that
	// couldn't fit withing squads will then create new squads.
	std::set<AlliedSquadId> checkedSquads;

	bool foundUnit = false;
	// Iterate through all squads taking one unit each turn
	do {
		foundUnit = false;
		std::pair<BWAPI::Unit*, AlliedSquadId> currentUnit;
		currentUnit.first = NULL;

		// Is there a not checked unit with a not checked squad?
		std::map<BWAPI::Unit*, AlliedSquadId>::const_iterator unitIt = mUnitsToCheck.begin();
		while (unitIt != mUnitsToCheck.end() && !foundUnit) {
			if (unitIt->second != AlliedSquadId::INVALID_KEY &&
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

			addCloseUnitsToSquad(unitIt->first, unitIt->second);
		}

	} while (!foundUnit);


	// Create squads for the rest of the units that went out of the squad's bounds
	while (!mUnitsToCheck.empty()) {
		// Create new squad
		AlliedSquad* pNewSquad = new AlliedSquad();
		addSquad(pNewSquad);
		// Note: addCloseUnitsToSquad, adds the unit to the squad automatically.

		BWAPI::Unit* currentUnit = mUnitsToCheck.begin()->first;
		setUnitAsChecked(mUnitsToCheck.begin());

		addCloseUnitsToSquad(currentUnit, pNewSquad->getId());
	}
}

void AlliedArmyManager::onConstantChanged(config::ConstantName constantName) {
	if (constantName == TO_CONSTANT_NAME(config::classification::squad::GRID_SQUARE_DISTANCE)) {
		recalculateLookupTable();
	}
}

void AlliedArmyManager::addUnit(BWAPI::Unit* pUnit) {
	// Note, this function does not update the squads, it simply add the unit to this class,
	// meaning in the next update phase the squads might be altered.
	mUnitSquad[pUnit] = AlliedSquadId::INVALID_KEY;

	//Position center = getGridPosition(pUnit);
	//if (Positions::Invalid == center) {
	//	ERROR_MESSAGE(false, "Invalid grid position for unit: " << pUnit->getType().getName());
	//	return;
	//}

	//addUnitToGrid(pUnit);


	//// Collect all squads withing include_distance range.
	//std::set<AlliedSquadId> squads;
	//std::pair<Position,Position> gridRange = getValidGridRange(center, GRID_RANGE_EXCLUDE_MAX);
	//for (int x = gridRange.first.x(); x <= gridRange.second.x(); ++x) {
	//	for (int y = gridRange.first.y(); y <= gridRange.second.y(); ++y) {
	//		
	//		// Find all 
	//		std::map<Unit*, bool>::iterator unitIt;
	//		for (unitIt = mGridUnits[x][y].begin(); unitIt != mGridUnits[x][y].end(); ++unitIt) {
	//			if (withinIncludeDistance(pUnit, unitIt->first)) {
	//				squads.insert(mUnitSquad[unitIt->first]);
	//			}
	//		}
	//	}
	//}
	//

	//// Squads found
	//if (!squads.empty()) {
	//	
	//	// All units within the same squad, add unit to squad
	//	if (squads.size() == 1) {
	//		if (NULL != mSquads[*squads.begin()]) {
	//			mSquads[*squads.begin()]->addUnit(pUnit);
	//		}
	//	}
	//	// Else Multiple squads, merge the squads
	//	else  {
	//		// Remove first 
	//		AlliedSquadId nextToMerge = *squads.begin();
	//		squads.erase(squads.begin());

	//		std::set<AlliedSquadId>::const_iterator squadIt;
	//		for (squadIt = squads.begin(); squadIt != squads.end(); ++squadIt) {
	//			nextToMerge = mergeSquads(nextToMerge, *squadIt);
	//		}

	//		// Add the unit to the final merged squad
	//		if (NULL != squads[nextToMerge]) {
	//			mSquads[nextToMerge]->addUnit(pUnit);
	//		}
	//	}

	//}
	//// Else no units found -> Create new squad
	//else  {
	//	AlliedSquad* pNewSquad = new AlliedSquad();
	//	addSquad(pNewSquad);

	//	pNewSquad->addUnit(pUnit);
	//}
}

void AlliedArmyManager::removeUnit(BWAPI::Unit* pUnit) {
	// Note, this function does not update the squads, it simply removes the unit from the squad
	// and this class, meaning in the next update phase the squads might be altered.
	std::map<Unit*, AlliedSquadId>::iterator unitIt = mUnitSquad.find(pUnit);
	if (unitIt != mUnitSquad.end() && unitIt->second.isValid()) {
		mSquads[unitIt->second]->removeUnit(pUnit);
	}

	mUnitSquad.erase(unitIt);
}

void AlliedArmyManager::addCloseUnitsToSquad(BWAPI::Unit* pUnit, AlliedSquadId squadId) {
	// Change squad if needed
	AlliedSquadId oldSquadId = mUnitSquad[pUnit];
	if (oldSquadId != squadId) {
		if (oldSquadId.isValid()) {
			mSquads[oldSquadId]->removeUnit(pUnit);
		}
		mSquads[squadId]->addUnit(pUnit);
	}

	
	// Indirectly recursive, first check all then recursive afterwards
	// Queue and checked units that are 100% certain in range of exclude range.
	// Includes same square, squares with direct borders to the current square,
	// not diagonally squares.
	// Squads with different squad than old squad OR new squad checked later.
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
						// now
						AlliedSquadId currentSquadId = mUnitSquad[unitIt->first];
						if (currentSquadId == oldSquadId || currentSquadId == squadId) {
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
			if (isSameOrBorderGridPosition(center, Position(x,y)) == false) {

				// Iterate through all units within this square
				std::map<Unit*, bool>::iterator unitIt;
				for (unitIt = mGridUnits[x][y].begin(); unitIt != mGridUnits[x][y].end(); ++unitIt) {
					// Only parse not checked units
					if (unitIt->second == false) {
						
						AlliedSquadId currentSquadId = mUnitSquad[unitIt->first];

						// Same squad, check exclude_distance
						if (currentSquadId == oldSquadId || currentSquadId == squadId) {
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

void AlliedArmyManager::setUnitAsChecked(BWAPI::Unit* pUnit) {
	const TilePosition& unitPos = pUnit->getTilePosition();
	mGridUnits[unitPos.x()][unitPos.y()][pUnit] = true;
	mUnitsToCheck.erase(pUnit);
}

std::map<BWAPI::Unit*, AlliedSquadId>::const_iterator AlliedArmyManager::setUnitAsChecked(const std::map<BWAPI::Unit*, AlliedSquadId>::const_iterator& unitIt) {
	const TilePosition& unitPos = unitIt->first->getTilePosition();
	mGridUnits[unitPos.x()][unitPos.y()][unitIt->first] = true;
	return mUnitsToCheck.erase(unitIt);
}

bool AlliedArmyManager::withinExcludeDistance(BWAPI::Unit* pUnitA, BWAPI::Unit* pUnitB) const {
	return getSquaredDistance(pUnitA->getTilePosition(), pUnitB->getTilePosition()) <=
		config::classification::squad::EXCLUDE_DISTANCE_SQUARED;
}

bool AlliedArmyManager::withinIncludeDistance(BWAPI::Unit* pUnitA, BWAPI::Unit* pUnitB) const {
	return getSquaredDistance(pUnitA->getTilePosition(), pUnitB->getTilePosition()) <=
		config::classification::squad::INCLUDE_DISTANCE_SQUARED;
}

BWAPI::Position AlliedArmyManager::getGridPosition(BWAPI::Unit* pUnit) const {
	TilePosition unitPos = BWAPI::TilePositions::Invalid;

	// Not loaded
	if (!pUnit->isLoaded()) {
		unitPos = pUnit->getTilePosition();
	} else {
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

	Position gridPos = Positions::Invalid;
	if (unitPos.x() >= 0 && unitPos.x() < static_cast<int>(mLookupTableGridPosition.size()) &&
		unitPos.y() >= 0 && unitPos.y() < static_cast<int>(mLookupTableGridPosition[0].size()))
	{
		gridPos = mLookupTableGridPosition[unitPos.x()][unitPos.y()];
	} else {
		ERROR_MESSAGE(false, "Invalid unit position: " << unitPos);
	}

	return gridPos;
}

bool AlliedArmyManager::isSameOrBorderGridPosition(const BWAPI::Position& centerPosition, const BWAPI::Position& checkPosition) const {
	// Only same or border if maximum one coordinate is from the center,
	// i.e. 0,0 center, 0,1, 1,0, -1,0, 0,-1 returns true
	int xDiff = abs(centerPosition.x() - checkPosition.x());
	int yDiff = abs(centerPosition.y() - checkPosition.y());

	return ((xDiff <= 1 && yDiff == 0) || (yDiff <= 1 && xDiff == 0));
}

std::pair<Position, Position> AlliedArmyManager::getValidGridRange(const BWAPI::Position centerPosition, int range) const {
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

void AlliedArmyManager::recalculateLookupTable() {
	int gridSquareDistance = config::classification::squad::GRID_SQUARE_DISTANCE;

	mLookupTableGridPosition.clear();
	mLookupTableGridPosition.resize(BWAPI::Broodwar->mapWidth());

	for (int x = 0; x < BWAPI::Broodwar->mapWidth(); x++) {
		mLookupTableGridPosition[x].resize(BWAPI::Broodwar->mapHeight());
		for (int y = 0; y < BWAPI::Broodwar->mapHeight(); y++) {
			mLookupTableGridPosition[x][y] = BWAPI::Position(x / gridSquareDistance, y / gridSquareDistance);
		}
	}
}

void AlliedArmyManager::disbandEmptySquads() {
	for (size_t i = 0; i < mSquads.size(); ++i) {
		if (NULL != mSquads[i]) {
			if (mSquads[i]->isEmpty()) {
				mSquads[i].reset();
			}
		}
	}
}

void AlliedArmyManager::setBigSquad() {
	// Save biggest squad and set all squads as small
	shared_ptr<AlliedSquad> biggestSquad;
	int cBiggestSize = 0;
	for (size_t i = 0; i < mSquads.size(); ++i) {
		if (NULL != mSquads[i]) {
			mSquads[i]->setBig(false);

			int currentSupplyCount = mSquads[i]->getSupplyCount();
			if (currentSupplyCount > cBiggestSize) {
				biggestSquad = mSquads[i];
				cBiggestSize = currentSupplyCount;
			}
		}
	}


	if (NULL != biggestSquad) {
		biggestSquad->setBig(true);
	}
}

void AlliedArmyManager::addSquad(AlliedSquad* pSquad) {
	mSquads[pSquad->getId()] = std::tr1::shared_ptr<AlliedSquad>(pSquad);
}

void AlliedArmyManager::removeSquad(AlliedSquadId squadId) {
	mSquads[squadId].reset();
}

void AlliedArmyManager::addUnitToGrid(BWAPI::Unit* pUnit) {
	Position gridPos = getGridPosition(pUnit);

	if (gridPos != Positions::Invalid) {
		mGridUnits[gridPos.x()][gridPos.y()][pUnit] = false;
	}
}