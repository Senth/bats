#include "AlliedSquad.h"
#include "Helper.h"
#include "ExplorationManager.h"
#include "GameTime.h"
#include "AlliedArmyManager.h"

using namespace bats;
using BWAPI::TilePosition;

utilities::KeyHandler<_AlliedSquadType>* AlliedSquad::mpsKeyHandler = NULL;
int AlliedSquad::mcsInstances = 0;
bats::ExplorationManager* AlliedSquad::mpsExplorationManager = NULL;
GameTime* AlliedSquad::mpsGameTime = NULL;

const int MAX_KEYS = 1000;

AlliedSquad::AlliedSquad(bool big) : mId(AlliedSquadId::INVALID_KEY) {
	mBig = big;
	mState = State_Idle;

	if (mpsExplorationManager == NULL) {
		mpsExplorationManager = bats::ExplorationManager::getInstance();
	}

	if (mpsGameTime == NULL) {
		mpsGameTime = GameTime::getInstance();
	}

	if (mcsInstances == 0) {
		utilities::KeyHandler<_AlliedSquadType>::init(MAX_KEYS);
		mpsKeyHandler = utilities::KeyHandler<_AlliedSquadType>::getInstance();
	}
	mcsInstances++;

	mId = mpsKeyHandler->allocateKey();

	// Add listener
	config::addOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::MEASURE_TIME), this);

	mLastUpdate = 0.0;
}

AlliedSquad::~AlliedSquad() {
	// Remove listener
	config::removeOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::MEASURE_TIME), this);
	
	mcsInstances--;

	mpsKeyHandler->freeKey(mId);

	// Delete KeyHandler if no squads are available
	if (mcsInstances == 0) {
		SAFE_DELETE(mpsKeyHandler);
	}
}

bool AlliedSquad::isBig() const {
	return mBig;
}

void AlliedSquad::setBig(bool big) {
	mBig = big;
}

const std::vector<BWAPI::Unit*> AlliedSquad::getUnits() const {
	return mUnits;
}

int AlliedSquad::getSupplyCount() const {
	int cSupply = 0;
	for (size_t i = 0; i < mUnits.size(); ++i) {
		cSupply += mUnits[i]->getType().supplyRequired();
	}

	return cSupply;
}

size_t AlliedSquad::getUnitCount() const {
	return mUnits.size();
}

bool AlliedSquad::isEmpty() const {
	return mUnits.size() == 0;
}

void AlliedSquad::addUnit(BWAPI::Unit* pUnit) {
	mUnits.push_back(pUnit);
}

void AlliedSquad::removeUnit(BWAPI::Unit* pUnit) {
	std::vector<BWAPI::Unit*>::iterator it = mUnits.begin();
	bool found = false;
	while (it != mUnits.end() && !found) {
		if (*it == pUnit) {
			found = true;
			mUnits.erase(it);
		} else {
			++it;
		}
	}
}

AlliedSquadId AlliedSquad::getId() const {
	return mId;
}

void AlliedSquad::onConstantChanged(config::ConstantName constanName) {
	// measure_time
	if (constanName == TO_CONSTANT_NAME(config::classification::squad::MEASURE_TIME)) {
		// If less then erase those at the back
		if (config::classification::squad::MEASURE_TIME < mCenter.size()) {
			mCenter.resize(config::classification::squad::MEASURE_TIME);
		}
		if (config::classification::squad::MEASURE_TIME < mHomeDistances.size()) {
			mHomeDistances.resize(config::classification::squad::MEASURE_TIME);
		}
		if (config::classification::squad::MEASURE_TIME < mEnemyDistances.size()) {
			mEnemyDistances.resize(config::classification::squad::MEASURE_TIME);
		}
	}
}

AlliedSquad::States AlliedSquad::getState() const {
	return mState;
}

void AlliedSquad::update() {
	// Skip if no units in squad
	if (mUnits.empty()) {
		return;
	}

	// Check if it has passed one second
	if (mpsGameTime->getElapsedTime() - mLastUpdate < 1.0) {
		return;
	}
	mLastUpdate = mpsGameTime->getElapsedTime();


	updateCenter();
	updateClosestDistances();


	// Retreating
	if (isRetreating()) {
		mState = State_Retreating;
	}
	// Attacking
	else if (isAttacking()) {
		mState = State_Attacking;
	}
	// Moving to attack
	else if (isMovingToAttack()) {
		mState = State_MovingToAttack;
	}
	// Stopped moving to attack
	else if (hasAttackHalted()) {
		mState = State_AttackHalted;
	}
	// else - Idle
	else {
		mState = State_Idle;
	}
}

int AlliedSquad::getMaxKeys() {
	return MAX_KEYS;
}

BWAPI::TilePosition AlliedSquad::getDirection() const {
	if (mCenter.size() < config::classification::squad::MEASURE_TIME) {
		return BWAPI::TilePositions::Invalid;
	} else {
		return mCenter.front() - mCenter.back();
	}
}

double AlliedSquad::getDistanceTraveledSquared() const {
	if (mCenter.size() < config::classification::squad::MEASURE_TIME) {
		return BWAPI::TilePositions::Invalid;
	} else {
		return getSquaredDistance(mCenter.front(), mCenter.back());
	}
}

bool AlliedSquad::isMovingToAttack() const {
	// Skip if we haven't all readings
	if (config::classification::squad::MEASURE_TIME != mCenter.size()) {
		return false;
	}

	// Moved minimum x tiles
	if (getDistanceTraveledSquared() < config::classification::squad::MOVED_TILES_MIN_SQUARED) {
		return false;
	}


	// At least attack_percent_away_min % away from our structures
	int enemyDistance = mEnemyDistances.front();
	int alliedDistance = mHomeDistances.front();

	// Compare
	int totalDistance = enemyDistance + alliedDistance;
	double fractionDistanceFromOurBases = static_cast<double>(alliedDistance) / static_cast<double>(totalDistance);
	if (fractionDistanceFromOurBases < config::classification::squad::ATTACK_FRACTION_AWAY_MIN) {
		return false;
	}


	// Moving towards enemy base
	// If the squad increases the distance from home, or decreases the distance to the enemy
	// it is treated as it's moving towards the enemy base. Note in code this will be negated because
	// the if should be false
	if (mHomeDistances.front() < mHomeDistances.back() &&
		mEnemyDistances.front() > mEnemyDistances.back())
	{
		return false;
	}


	return true;
}

bool AlliedSquad::isRetreating() const {
	// Skip if we haven't all readings
	if (config::classification::squad::MEASURE_TIME != mCenter.size()) {
		return false;
	}

	// Moved minimum x tiles
	if (getDistanceTraveledSquared() < config::classification::squad::MOVED_TILES_MIN_SQUARED) {
		return false;
	}


	// At least retreat_percent_away_min % away form our structures
	int enemyDistance = mEnemyDistances.front();
	int alliedDistance = mHomeDistances.front();

	// Compare
	int totalDistance = enemyDistance + alliedDistance;
	double fractionDistanceFromOurBases = static_cast<double>(alliedDistance) / static_cast<double>(totalDistance);
	if (fractionDistanceFromOurBases < config::classification::squad::RETREAT_FRACTION_AWAY_MIN) {
		return false;
	}


	// Shall move away from enemy structures, will this work?
	if (mEnemyDistances.front() < mEnemyDistances.back()) {
		return false;
	}


	return false;
}

bool AlliedSquad::isAttacking() const {
	// At least one of the squad units is attacking something
	bool isAttacking = false;
	size_t i = 0;
	while (!isAttacking && i < mUnits.size()) {
		if (mUnits[i]->isAttacking() &&
			mUnits[i]->getOrderTarget() != NULL &&
			BWAPI::Broodwar->self()->isEnemy(mUnits[i]->getOrderTarget()->getPlayer()))
		{
			isAttacking = true;
		}
		++i;
	}

	if (!isAttacking) {
		return false;
	}


	// At least retreat_percent_away_min % away form our structures
	int enemyDistance = mEnemyDistances.front();
	int alliedDistance = mHomeDistances.front();

	// Compare
	int totalDistance = enemyDistance + alliedDistance;
	double fractionDistanceFromOurBases = static_cast<double>(alliedDistance) / static_cast<double>(totalDistance);
	if (fractionDistanceFromOurBases < config::classification::squad::RETREAT_FRACTION_AWAY_MIN) {
		return false;
	}

	return false;
}

bool AlliedSquad::hasAttackHalted() const {
	// Skip if we haven't all readings
	if (config::classification::squad::MEASURE_TIME != mCenter.size()) {
		return false;
	}

	// Moved MAXIMUM x tiles
	if (getDistanceTraveledSquared() >= config::classification::squad::MOVED_TILES_MIN_SQUARED) {
		return false;
	}


	// At least retreat_percent_away_min % away form our structures
	int enemyDistance = mEnemyDistances.front();
	int alliedDistance = mHomeDistances.front();

	// Compare
	int totalDistance = enemyDistance + alliedDistance;
	double fractionDistanceFromOurBases = static_cast<double>(alliedDistance) / static_cast<double>(totalDistance);
	if (fractionDistanceFromOurBases < config::classification::squad::RETREAT_FRACTION_AWAY_MIN) {
		return false;
	}

	return false;
}

void AlliedSquad::updateCenter() {
	TilePosition center(0,0);

	for (size_t i = 0; i < mUnits.size(); ++i) {
		TilePosition unitPos = mUnits[i]->getTilePosition();
		if (unitPos.isValid()) {
			center += unitPos;
		}
	}

	if (mUnits.size() > 0) {
		center.x() /= mUnits.size();
		center.y() /= mUnits.size();
	}

	mCenter.push_front(center);

	// Delete the oldest (if full)
	if (config::classification::squad::MEASURE_TIME < mCenter.size()) {
		mCenter.pop_back();
	}

}

void AlliedSquad::updateClosestDistances() {
	// Save distances from closest allied and enemy structures
	int enemyDistance = mpsExplorationManager->getClosestSpottedBuilding(mCenter.front()).second;
	int alliedDistance = getClosestAlliedStructure(mCenter.front()).second;

	mEnemyDistances.push_front(enemyDistance);
	mHomeDistances.push_front(alliedDistance);

	// Delete the oldest (if full)
	if (config::classification::squad::MEASURE_TIME < mHomeDistances.size()) {
		mHomeDistances.pop_back();
	}
	if (config::classification::squad::MEASURE_TIME < mEnemyDistances.size()) {
		mEnemyDistances.pop_back();
	}
}

void AlliedSquad::printInfo() {
	// Skip if not turned on
	if (config::debug::GRAPHICS_VERBOSITY == config::debug::GraphicsVerbosity_Off ||
		config::debug::classes::ALLIED_SQUAD == false) {
		return;
	}


	// Low
	// Print id, state, number of units and number of supplies.
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Low) {
		if (!mCenter.empty()) {
			BWAPI::Position squadCenterOnMap = BWAPI::Position(mCenter.front());
			BWAPI::Broodwar->drawTextMap(
					squadCenterOnMap.x(), squadCenterOnMap.y(),
					"\x02Id: \x04%i\n\x02State: \x04%s\n\x02Units: \x04%i\n\x02Supplies: \x04%g",
					mId, getStateString(), getUnitCount(),
					static_cast<double>(getSupplyCount()) * 0.5
				);
		}
	}


	// Medium
	// Draw line from the front and back center, display the length of this line
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
		/// @todo medium draw line and length of line
	}
}

std::string AlliedSquad::getStateString() const {
	switch (mState) {
		case State_AttackHalted:
			return "Attack halted";

		case State_Attacking:
			return "Attacking";

		case State_Idle:
			return "Idle";

		case State_MovingToAttack:
			return "Moving to attack";

		case State_Retreating:
			return "Retreating";

		default:
			return "Unknown state";
	}
}