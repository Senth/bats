#include "AlliedArmyManager.h"
#include "Utilities/Logger.h"
#include "AlliedSquad.h"
#include <cstdlib> // For NULL
#include <BWAPI/Game.h>
#include <cassert>

using namespace bats;
using namespace BWAPI;

AlliedArmyManager* AlliedArmyManager::mpsInstance = NULL;

AlliedArmyManager::AlliedArmyManager() {
	recalculateLookupTable();

	config::addOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::GRID_SQUARE_DISTANCE), this);

	/// @todo
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
	rearrangeSquads();

	/// @todo disband empty squads

	/// @todo check if big squad still is big

	// Update all squads
	for (size_t i = 0; i < mSquads.size(); ++i) {
		if (NULL != mSquads[i]) {
			mSquads[i]->update();
		}
	}
}

void AlliedArmyManager::rearrangeSquads() {
	std::map<BWAPI::Unit*, AlliedSquadId> unitsToCheck = mUnitSquad;


	// Create the grid with units
	// 2D grid, each square can hold any number of units (3rd vector), each unit has information
	// if they have been checked or not (bool in pair).
	std::vector<std::vector<std::vector<std::pair<Unit*, bool>>>> gridUnits;

	// Initialize size of grid
	gridUnits.resize(mLookupTableGridPosition.size());
	for (size_t x = 0; x < gridUnits.size(); ++x) {
		gridUnits[x].resize(mLookupTableGridPosition[0].size());
	}

	// Add all units to the grid
	std::map<BWAPI::Unit*, AlliedSquadId>::const_iterator unitIt;
	for (unitIt = unitsToCheck.begin(); unitIt != unitsToCheck.end(); ++unitIt) {
		TilePosition unitPos;

		// Not loaded
		if (!unitIt->first->isLoaded()) {
			unitPos = unitIt->first->getTilePosition();
		} else {
			// Get position of the transport, this is needed because the position of the units
			// are not updated while loaded.
			Unit* pTransport = unitIt->first->getTransport();
			assert(NULL != pTransport);
			if (NULL != pTransport) {
				unitPos = pTransport->getTilePosition();
			} else {
				ERROR_MESSAGE(false, "Unit is loaded, but doesn't have a transport!");
				unitPos = unitIt->first->getTilePosition();
			}
		}

		if (unitPos.x() >= 0 && unitPos.x() < static_cast<int>(gridUnits.size()) &&
			unitPos.y() >= 0 && unitPos.y() < static_cast<int>(gridUnits[0].size()))
		{
			gridUnits[unitPos.x()][unitPos.y()].push_back(std::make_pair(unitIt->first, false));
		} else {
			ERROR_MESSAGE(false, "Invalid unit position: " << unitPos);
		}
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
		std::map<BWAPI::Unit*, AlliedSquadId>::const_iterator unitIt = unitsToCheck.begin();
		while (unitIt != unitsToCheck.end() && !foundUnit) {
			if (unitIt->second != AlliedSquadId::INVALID_KEY &&
				checkedSquads.count(unitIt->second) == 0)
			{
				foundUnit = true;
				currentUnit = (*unitIt);

				// Remove, i.e. set as checked
				unitIt = unitsToCheck.erase(unitIt);
			} else {
				++unitIt;
			}
		}


		if (foundUnit) {
			// Set squad as checked
			checkedSquads.insert(currentUnit.second);

			/// @todo group unit with other close units
		}

	} while (!foundUnit);


	// Create squads for the rest of the units that went out of the squad's bounds
	while (!unitsToCheck.empty()) {
		// Create new squad
		AlliedSquad* pNewSquad = new AlliedSquad();
		addSquad(pNewSquad);
	}
}

void AlliedArmyManager::onConstantChanged(config::ConstantName constantName) {
	if (constantName == TO_CONSTANT_NAME(config::classification::squad::GRID_SQUARE_DISTANCE)) {
		recalculateLookupTable();
	}
}

BWAPI::Position AlliedArmyManager::getGridPosition(BWAPI::Unit* pUnit) const {
	return mLookupTableGridPosition[pUnit->getTilePosition().x()][pUnit->getTilePosition().y()];
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

void AlliedArmyManager::addSquad(AlliedSquad* pSquad) {
	mSquads[pSquad->getId()] = std::tr1::shared_ptr<AlliedSquad>(pSquad);
}

void AlliedArmyManager::removeSquad(AlliedSquadId squadId) {
	mSquads[squadId].reset();
}