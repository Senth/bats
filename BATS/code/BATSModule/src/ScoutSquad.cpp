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
	Squad(units, avoidEnemy, true, unitComposition),
	mAvoidEnemy(avoidEnemy){
		//activate();
}

ScoutSquad::~ScoutSquad(){
}

bool ScoutSquad::createGoal(){
	TilePosition nGoal = ExplorationManager::getInstance()->getNextToExplore(getThis());
	setGoalPosition(nGoal);
	return true;
}

void ScoutSquad::computeSquadSpecificActions(){	
	
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

Squad::GoalStates ScoutSquad::checkGoalState() const{
	TilePosition currentPos = getCenter();
	TilePosition goal = getGoal();
	if (goal == BWAPI::TilePositions::Invalid) {
		return Squad::GoalState_Succeeded;
	} else if (isCloseTo(goal)) {
		return Squad::GoalState_Succeeded;
	}

	/// @todo set scout as failed when it finds enemies in its way...

	return Squad::GoalState_NotCompleted;
}