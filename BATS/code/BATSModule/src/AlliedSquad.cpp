#include "AlliedSquad.h"
#include "UnitHelper.h"
#include "ExplorationManager.h"
//#include "AlliedArmyManager.h"
#include "GameTime.h"
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace bats;
using namespace BWAPI;
using namespace std;

bats::ExplorationManager* AlliedSquad::mpsExplorationManager = NULL;

AlliedSquad::AlliedSquad(bool frontalAttack) {
	mFrontattack = frontalAttack;
	mState = State_IdleInBase;

	if (mpsExplorationManager == NULL) {
		mpsExplorationManager = bats::ExplorationManager::getInstance();
	}

	mAttackLast = mpsGameTime->getElapsedTime() - config::classification::squad::ATTACK_TIMEOUT;
	mUnderAttackLast = mpsGameTime->getElapsedTime() - config::classification::squad::ATTACK_TIMEOUT;
	mRetreatStartedTime = 0.0;
	mRetreatStartTestTime = 0.0;
	mRetreatedLastCall = false;
}

AlliedSquad::~AlliedSquad() {
	// Does nothing
}

bool AlliedSquad::isFrontalAttack() const {
	return mFrontattack;
}

void AlliedSquad::setFrontalAttack(bool frontalAttack) {
	mFrontattack = frontalAttack;
}

AlliedSquad::States AlliedSquad::getState() const {
	return mState;
}

void AlliedSquad::updateDerived() {
	updateClosestDistances();


	// Attacking
	if (isAttacking()) {
		mState = State_Attacking;
	}
	// Retreating
	else if (isRetreating()) {
		mState = State_Retreating;
	}
	// Moving to attack
	else if (isMovingToAttack()) {
		mState = State_MovingToAttack;
	}
	// Stopped moving to attack
	else if (hasAttackHalted()) {
		mState = State_IdleOutsideBase;
	}
	// else - Idle
	else {
		mState = State_IdleInBase;
	}
}

bool AlliedSquad::isUnderAttack() const {
	// Check if an enemy unit has one of the squads as targets
	// Enemy players
	const set<Player*>& enemies = Broodwar->enemies();
	set<Player*>::const_iterator enemyIt;
	for (enemyIt = enemies.begin(); enemyIt != enemies.end(); ++enemyIt) {
		// Enemy units
		const set<Unit*>& enemyUnits = (*enemyIt)->getUnits();
		set<Unit*>::const_iterator enemyUnitIt;
		for (enemyUnitIt = enemyUnits.begin(); enemyUnitIt != enemyUnits.end(); ++enemyUnitIt) {
			// Does the target belongs to this squad?
			Unit* pEnemyTarget = (*enemyUnitIt)->getTarget();
			if (pEnemyTarget != NULL && belongsToThisSquad(pEnemyTarget)) {
				return true;
			}
		}
	}

	return false;
}

void AlliedSquad::onConstantChanged(config::ConstantName constanName) {
	PlayerSquad::onConstantChanged(constanName);

	// measure_time
	if (constanName == TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE)) {
		// If less then erase those at the back
		if (config::classification::squad::MEASURE_SIZE < mAlliedDistances.size()) {
			mAlliedDistances.resize(config::classification::squad::MEASURE_SIZE);
		}
		if (config::classification::squad::MEASURE_SIZE < mEnemyDistances.size()) {
			mEnemyDistances.resize(config::classification::squad::MEASURE_SIZE);
		}
	}
}

bool AlliedSquad::isMovingToAttack() const {
	// Skip if we haven't all readings
	if (!isMeasureFull()) {
		return false;
	}

	// Moved minimum x tiles
	if (getDistanceTraveledSquared() < config::classification::squad::MOVED_TILES_MIN_SQUARED) {
		return false;
	}


	// Target at least away_distance from allied structures
	TilePosition targetPosition = getTargetPosition();
	if (targetPosition != TilePositions::Invalid) {
		pair<BWAPI::Unit*, int> closestAlliedStructure = UnitHelper::getClosestAlliedStructure(targetPosition);
		if (closestAlliedStructure.second < config::classification::squad::AWAY_DISTANCE_SQUARED)
		{
			return false;
		}
	}
	// Else - if no target -> squad needs to be at least away distance from allied structures
	else {
		if (mAlliedDistances.empty() ||
			mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED)
		{
			return false;
		}
	}

	return true;
}

bool AlliedSquad::hasRetreatTimedout() const {
	return mRetreatStartedTime + config::classification::squad::RETREAT_TIMEOUT
		<=
		mpsGameTime->getElapsedTime();
}

bool AlliedSquad::isRetreating() const {
	bool retreating = false;

	if (mState == State_Retreating && !hasRetreatTimedout()) {
		retreating = true;
	} else {
		// Retreating, or not safe
		if (mState == State_Retreating || isUnderAttack()) {
			if (isRetreatingFrame()) {
				mRetreatedLastCall = true;
				mRetreatStartedTime = mpsGameTime->getElapsedTime();
				retreating = true;
			} else {
				mRetreatedLastCall = false;
			}
		}
		// Else - Squad is safe
		else {
			if (isRetreatingFrame()) {
				if (mRetreatedLastCall) {
					if (mRetreatStartTestTime + config::classification::squad::RETREAT_TIME_WHEN_SAFE
						<
						mpsGameTime->getElapsedTime())
					{
						mRetreatStartedTime = mpsGameTime->getElapsedTime();
						retreating = true;
					}
				}
				// Set starting to try retreating
				else {
					mRetreatedLastCall = true;
					mRetreatStartTestTime = mpsGameTime->getElapsedTime();
				}
			} else {
				mRetreatedLastCall = false;
			}
		}
	}


	return retreating;
}

bool AlliedSquad::isRetreatingFrame() const {
	// Skip if we haven't all readings
	if (!isMeasureFull()) {
		return false;
	}


	// Moved minimum x tiles
	if (getDistanceTraveledSquared() < config::classification::squad::MOVED_TILES_MIN_SQUARED) {
		return false;
	}


	// At least away_distance from allied structures
	if (mAlliedDistances.empty() ||
		mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED)
	{
		return false;
	}


	// If we have spotted enemy structures, squad hall move away from enemy structures.
	// Else use allied structures and we shall move towards allied structures.
	if (!mEnemyDistances.empty()) {
		if (mEnemyDistances.front() < mEnemyDistances.back()) {
			return false;
		}
	} else if (mAlliedDistances.empty() || mAlliedDistances.front() > mAlliedDistances.back()) {
		return false;
	}


	return true;
}

bool AlliedSquad::isAttacking() const {
	bool bFrameAttacking = isAttackingFrame();

	if (bFrameAttacking) {
		mAttackLast = mpsGameTime->getElapsedTime();
	}


	if (mAttackLast + config::classification::squad::ATTACK_TIMEOUT > mpsGameTime->getElapsedTime()) {
		return true;
	} else {
		return false;
	}
}

bool AlliedSquad::isAttackingFrame() const {
	// At least one of the squad units is attacking something
	bool isAttacking = false;

	const vector<const Unit*>& units = getUnits();
	size_t i = 0;
	while (!isAttacking && i < units.size()) {
		if (units[i]->getOrderTarget() != NULL &&
			BWAPI::Broodwar->self()->isEnemy(units[i]->getOrderTarget()->getPlayer()))
		{
			isAttacking = true;
		}
		++i;
	}

	if (!isAttacking) {
		return false;
	}

	
	// At least away_distance from allied structures
	if (mAlliedDistances.empty() ||
		mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED)
	{
		return false;
	}


	return true;
}

bool AlliedSquad::hasAttackHalted() const {
	// Skip if we haven't all readings
	if (!isMeasureFull()) {
		return false;
	}


	// Moved MAXIMUM x tiles
	if (getDistanceTraveledSquared() >= config::classification::squad::MOVED_TILES_MIN_SQUARED) {
		return false;
	}


	// At least away_distance from allied structures
	if (mAlliedDistances.empty() ||
		mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED)
	{
		return false;
	}


	return true;
}

void AlliedSquad::updateClosestDistances() {
	// Save distances from closest allied and enemy structures
	pair<TilePosition, int> enemyStructure = mpsExplorationManager->getClosestSpottedBuilding(getCenter());
	pair<Unit*, int> alliedStructure = UnitHelper::getClosestAlliedStructure(getCenter());

	DEBUG_MESSAGE(utilities::LogLevel_Finest, "AlliedSquad::updateClosestDistance() | id: " <<
		getId() << ", center: " << getCenter() << ", Enemy: " << enemyStructure.first <<
		" distance: " << enemyStructure.second << ", Allied: " <<
		alliedStructure.first->getTilePosition() << " distance: " << alliedStructure.second);

	if (enemyStructure.first.isValid()) {
		mEnemyDistances.push_front(enemyStructure.second);
	}
	if (alliedStructure.first->getTilePosition().isValid()){
		mAlliedDistances.push_front(alliedStructure.second);
	}

	// Delete the oldest (if full)
	if (config::classification::squad::MEASURE_SIZE < mAlliedDistances.size()) {
		mAlliedDistances.pop_back();
	}
	if (config::classification::squad::MEASURE_SIZE < mEnemyDistances.size()) {
		mEnemyDistances.pop_back();
	}
}

std::string AlliedSquad::getDebugString() const {
	double alliedDistance = 0.0;
	if (!mAlliedDistances.empty()) {
		alliedDistance = sqrt(static_cast<double>(mAlliedDistances.front()));
	}

	stringstream ss;
	ss << setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "State: " << getStateString() << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Distance: " << alliedDistance;

	return PlayerSquad::getDebugString() + ss.str();
}

bool AlliedSquad::isDebugOn() const {
	return config::debug::modules::ALLIED_SQUAD;
}

std::string AlliedSquad::getStateString() const {
	switch (mState) {
		case State_IdleOutsideBase:
			return "Attack halted";

		case State_Attacking:
			return "Attacking";

		case State_IdleInBase:
			return "Idle";

		case State_MovingToAttack:
			return "Moving to attack";

		case State_Retreating:
			return "Retreating";

		default:
			return "Unknown state";
	}
}

bool AlliedSquad::isActive() const {
	return mState != State_IdleInBase;
}