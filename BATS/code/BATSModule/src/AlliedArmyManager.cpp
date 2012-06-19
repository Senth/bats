#include "AlliedArmyManager.h"
#include <cstdlib> // For NULL

using namespace bats;

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
	/// @todo
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
	/// @todo
}