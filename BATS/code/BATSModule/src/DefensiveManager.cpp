#include "DefensiveManager.h"
#include "UnitManager.h"
#include <BWAPI/Unit.h>
#include <cstdlib> // For NULL

using namespace bats;
using namespace BWAPI;
using namespace std;

DefensiveManager* DefensiveManager::mpsInstance = NULL;

DefensiveManager::DefensiveManager() {
	mpUnitManager = NULL;

	mpUnitManager = UnitManager::getInstance();
}

DefensiveManager::~DefensiveManager() {
	mpsInstance = NULL;
}

DefensiveManager* DefensiveManager::getInstance() {
	if (NULL == mpsInstance) {
		mpsInstance = new DefensiveManager();
	}
	return mpsInstance;
}

void DefensiveManager::update() {
	/// @todo stub
}

vector<Unit*> DefensiveManager::getAllFreeUnits() {
	/// @todo stub
	return vector<Unit*>();
}

bool DefensiveManager::isUnderAttack() const {
	/// @todo stub
	return false;
}