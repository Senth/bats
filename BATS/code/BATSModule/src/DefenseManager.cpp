#include "DefenseManager.h"
#include "UnitManager.h"
#include <BWAPI/Unit.h>
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
	/// @todo stub
}

vector<Unit*> DefenseManager::getAllFreeUnits() {
	/// @todo stub
	return vector<Unit*>();
}

bool DefenseManager::isUnderAttack() const {
	/// @todo stub
	return false;
}