#include "DefenseManager.h"
#include "UnitManager.h"
#include "SquadManager.h"
#include "DefenseMoveSquad.h"
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

	updateDefendPositions();

	// Create DefenseMoveSquad if none exists
	//const vector<DefenseMoveSquadPtr>& moveSquads = mpSquadManager->getSquads<DefenseMoveSquad>();
	

	// Else - Set new wait position for the DefenseMoveSquad

	// Add free units to the defense squads, except transportations
	/// @todo Remove overlords
	
	/// @todo Does any DefenseHoldSquad need more units, i.e. not full.

	/// @todo is any defense point undefended, create DefenseHoldSquad for that position

	/// @todo All defense points are defended
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
	// Find the actual position

	/// @todo Were any positions removed, remove the DefenseHoldSquads in that case.

	/// @todo Were any positions

	/// 
}