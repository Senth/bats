#include "AlliedSquad.h"
#include "Helper.h"
#include "ExplorationManager.h"
#include "GameTime.h"
#include "AlliedArmyManager.h"

using namespace bats;
using namespace BWAPI;
using namespace std;

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
			it = mUnits.erase(it);
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
		if (config::classification::squad::MEASURE_TIME < mAlliedDistances.size()) {
			mAlliedDistances.resize(config::classification::squad::MEASURE_TIME);
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

	//// At least attack_percent_away_min % away from our structures
	//int enemyDistance = mEnemyDistances.front();
	//int alliedDistance = mAlliedDistances.front();

	//// Compare
	//int totalDistance = enemyDistance + alliedDistance;
	//double fractionDistanceFromOurBases = static_cast<double>(alliedDistance) / static_cast<double>(totalDistance);
	//if (fractionDistanceFromOurBases < config::classification::squad::ATTACK_FRACTION_AWAY_MIN) {
	//	return false;
	//}


	// At least away_distance from allied structures
	if (mAlliedDistances.empty() || mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED) {
		return false;
	}


	// Moving towards enemy base
	// If we haven't seen any enemy structure yet, only check if the distance from allies increases
	// Else enemy distance shall be decreased
	if (mAlliedDistances.empty()) {
		if (mAlliedDistances.front() < mAlliedDistances.back()) {
			return false;
		}
	} else if (mEnemyDistances.empty() || mEnemyDistances.front() > mEnemyDistances.back()) {
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


	// At least away_distance from allied structures
	if (mAlliedDistances.empty() || mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED) {
		return false;
	}

	//// At least retreat_percent_away_min % away form our structures
	//int enemyDistance = mEnemyDistances.front();
	//int alliedDistance = mAlliedDistances.front();

	//// Compare
	//int totalDistance = enemyDistance + alliedDistance;
	//double fractionDistanceFromOurBases = static_cast<double>(alliedDistance) / static_cast<double>(totalDistance);
	//if (fractionDistanceFromOurBases < config::classification::squad::RETREAT_FRACTION_AWAY_MIN) {
	//	return false;
	//}


	// If we have spotted enemy structures, squad hall move away from enemy structures.
	// Else use allied structures and we shall move towards allied structures.
	if (!mEnemyDistances.empty()) {
		if (mEnemyDistances.front() < mEnemyDistances.back()) {
			return false;
		}
	} else if (mAlliedDistances.empty() || mAlliedDistances.front() > mAlliedDistances.back()) {
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

	
	// At least away_distance from allied structures
	if (mAlliedDistances.empty() ||
		mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED)
	{
		return false;
	}


	//// At least retreat_percent_away_min % away form our structures
	//int enemyDistance = mEnemyDistances.front();
	//int alliedDistance = mAlliedDistances.front();

	//// Compare
	//int totalDistance = enemyDistance + alliedDistance;
	//double fractionDistanceFromOurBases = static_cast<double>(alliedDistance) / static_cast<double>(totalDistance);
	//if (fractionDistanceFromOurBases < config::classification::squad::RETREAT_FRACTION_AWAY_MIN) {
	//	return false;
	//}

	return true;
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


	// At least away_distance from allied structures
	if (mAlliedDistances.empty() || mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED) {
		return false;
	}

	//// At least retreat_percent_away_min % away form our structures
	//int enemyDistance = mEnemyDistances.front();
	//int alliedDistance = mAlliedDistances.front();

	//// Compare
	//int totalDistance = enemyDistance + alliedDistance;
	//double fractionDistanceFromOurBases = static_cast<double>(alliedDistance) / static_cast<double>(totalDistance);
	//if (fractionDistanceFromOurBases < config::classification::squad::RETREAT_FRACTION_AWAY_MIN) {
	//	return false;
	//}

	return true;
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
	pair<TilePosition, int> enemyStructure = mpsExplorationManager->getClosestSpottedBuilding(mCenter.front());
	pair<Unit*, int> alliedStructure = getClosestAlliedStructure(mCenter.front());

	DEBUG_MESSAGE(utilities::LogLevel_Finest, "AlliedSquad::updateClosestDistance() | id: " <<
		mId << ", center: " << mCenter.front() << ", Enemy: " << enemyStructure.first <<
		" distance: " << enemyStructure.second << ", Allied: " <<
		alliedStructure.first->getTilePosition() << " distance: " << alliedStructure.second);

	if (enemyStructure.first.isValid()) {
		mEnemyDistances.push_front(enemyStructure.second);
	}
	if (alliedStructure.first->getTilePosition().isValid()){
		mAlliedDistances.push_front(alliedStructure.second);
	}

	// Delete the oldest (if full)
	if (config::classification::squad::MEASURE_TIME < mAlliedDistances.size()) {
		mAlliedDistances.pop_back();
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

			string format =
				TextColors::LIGHT_BLUE + "Id: " + TextColors::WHITE + "%i\n" +
				TextColors::LIGHT_BLUE + "State: " + TextColors::WHITE + "%s\n" +
				TextColors::LIGHT_BLUE + "Units: " + TextColors::WHITE + "%i\n" +
				TextColors::LIGHT_BLUE + "Supplies: " + TextColors::WHITE + "%g\n" +
				TextColors::LIGHT_BLUE + "Distance: " + TextColors::WHITE + "%i";

			int alliedDistance = 0;
			if (!mAlliedDistances.empty()) {
				alliedDistance = mAlliedDistances.front();
			}

			BWAPI::Broodwar->drawTextMap(
				squadCenterOnMap.x(), squadCenterOnMap.y(),
				format.c_str(),
				mId,
				getStateString().c_str(),
				getUnitCount(),
				static_cast<double>(getSupplyCount()) * 0.5,
				alliedDistance
			);
		}
	}


	// Medium
	// Draw line from the front and back center, display the length of this line
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
		if (!mCenter.empty()) {
			pair<Position, Position> squadMovement = make_pair(Position(mCenter.front()), mCenter.back());
			
			// Length
			double length = (mCenter.front() - mCenter.back()).getLength();
			
			// Draw line
			Broodwar->drawLineMap(
				squadMovement.first.x(), squadMovement.first.y(),
				squadMovement.second.x(), squadMovement.second.y(),
				Colors::Grey
			);

			// Draw text in back of line
			Broodwar->drawTextMap(
				squadMovement.second.x(), squadMovement.second.y(),
				"\x10Length: %g",
				length
			);
			
		}
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