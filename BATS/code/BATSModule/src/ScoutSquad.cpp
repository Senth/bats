#include "ScoutSquad.h"
#include "ExplorationManager.h"

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
	// Does nothing
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
}

Squad::GoalStates ScoutSquad::checkGoalState() const{
	TilePosition currentPos = getCenter();
	TilePosition goal = getGoalPosition();
	if(isEnemyAttackUnitsWithinSight()){
		BWAPI::Broodwar->printf("Enemy detected");
		return Squad::GoalState_Succeeded;
	}
	if (goal == BWAPI::TilePositions::Invalid) {
	return Squad::GoalState_Succeeded;
	} else if (isCloseTo(goal)) {
		return Squad::GoalState_Succeeded;
	}

	/// @todo set scout as failed when it finds enemies in its way...

	return Squad::GoalState_NotCompleted;
}