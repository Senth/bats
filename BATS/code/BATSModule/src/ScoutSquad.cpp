#include "ScoutSquad.h"
#include "ExplorationManager.h"
#include "Utilities/Logger.h"

using namespace bats;
using BWAPI::TilePosition;
using namespace std::tr1;

ScoutSquad::ScoutSquad(
	const std::vector<UnitAgent*> units, 
	bool avoidEnemy,
	const UnitComposition& unitComposition)
	:	
	Squad(units, avoidEnemy, true, unitComposition)
{
	mNewEnemyOnDetect = true;
	mEnemyDetected = false;
}

ScoutSquad::~ScoutSquad(){
	// Does nothing
}

bool ScoutSquad::createGoal(){
	TilePosition nGoal = ExplorationManager::getInstance()->getNextToExplore(getThis());
	setGoalPosition(nGoal);
	return true;
}

void ScoutSquad::onGoalFailed(){
	createGoal();
}

void ScoutSquad::onGoalSucceeded(){
	createGoal();
}

std::string ScoutSquad::getName() const{
	return "ScoutSquad";
}

void ScoutSquad::updateDerived() {
	/// @todo move create goal, and check goal state here.

	/// @todo check if we can cloak the unit, or if its cloaked
	/// Don't mind enemies then if there are no detectors in range
	
	// Check for nearby enemies
	if (isEnemyAttackUnitsWithinSight()) {
		// If enemies were detected last time it means this are not new enemies
		if (mEnemyDetected) {
			mNewEnemyOnDetect = false;
		}

		mEnemyDetected = true;
	} else {
		mNewEnemyOnDetect = true;
		mEnemyDetected = false;
	}
}

Squad::GoalStates ScoutSquad::checkGoalState() const{
	TilePosition currentPos = getCenter();
	TilePosition goal = getGoalPosition();
	if(mNewEnemyOnDetect && mEnemyDetected){
		DEBUG_MESSAGE(utilities::LogLevel_Fine, "ScoutSquad: Enemy detected, changing goal");
		return Squad::GoalState_Failed;
	} else if (goal == BWAPI::TilePositions::Invalid) {
		return Squad::GoalState_Succeeded;
	} else if (isCloseTo(goal)) {
		return Squad::GoalState_Succeeded;
	}

	/// @todo set scout as failed when it finds enemies in its way...

	return Squad::GoalState_NotCompleted;
}