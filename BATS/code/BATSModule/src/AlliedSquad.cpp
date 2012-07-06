#include "AlliedSquad.h"
#include "Helper.h"
#include "ExplorationManager.h"
#include "GameTime.h"
#include "AlliedArmyManager.h"
#include <cmath>
#include <sstream>
#include <iomanip>

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

	mUpdateLast = 0.0;
	mAttackLast = mpsGameTime->getElapsedTime() - config::classification::squad::ATTACK_TIMEOUT;
	mUnderAttackLast = mpsGameTime->getElapsedTime() - config::classification::squad::ATTACK_TIMEOUT;
	mRetreatStartedTime = 0.0;
	mRetreatStartTestTime = 0.0;
	mRetreatedLastCall = false;
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
	if (mpsGameTime->getElapsedTime() - mUpdateLast < 1.0) {
		return;
	}
	mUpdateLast = mpsGameTime->getElapsedTime();


	updateCenter();
	updateClosestDistances();


	/// @todo check if under attack

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

bool AlliedSquad::isMovingToAttack() const {
	// Skip if we haven't all readings
	if (config::classification::squad::MEASURE_TIME != mCenter.size()) {
		return false;
	}

	// Moved minimum x tiles
	if (getDistanceTraveledSquared() < config::classification::squad::MOVED_TILES_MIN_SQUARED) {
		return false;
	}


	// Target at least away_distance from allied structures
	TilePosition targetPosition = getTargetPosition();
	if (targetPosition != TilePositions::Invalid) {
		pair<BWAPI::Unit*, int> closestAlliedStructure = getClosestAlliedStructure(targetPosition);
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
	if (config::classification::squad::MEASURE_TIME != mCenter.size()) {
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
	if (mAlliedDistances.empty() ||
		mAlliedDistances.front() < config::classification::squad::AWAY_DISTANCE_SQUARED)
	{
		return false;
	}


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

void AlliedSquad::printGraphicDebugInfo() {
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

			double alliedDistance = 0.0;
			if (!mAlliedDistances.empty()) {
				alliedDistance = sqrt(static_cast<double>(mAlliedDistances.front()));
			}

			stringstream ss;
			ss << TextColors::WHITE << left << setprecision(2) <<
				setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Id: " << mId << "\n" <<
				setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "State: " << getStateString() << "\n" <<
				setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Units: " << getUnitCount() << "\n" <<
				setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Supplies: " << getSupplyCount() << "\n" <<
				setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Distance: " << alliedDistance;


			BWAPI::Broodwar->drawTextMap(squadCenterOnMap.x(), squadCenterOnMap.y(), "%s", ss.str().c_str());
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
				Colors::Purple
			);

			int xOffset = -64;

			// Draw text in back of line
			Broodwar->drawTextMap(
				squadMovement.second.x() + xOffset, squadMovement.second.y(),
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

const TilePosition& AlliedSquad::getCenter() const {
	if (!mCenter.empty()) {
		return mCenter.front();
	} else {
		return TilePositions::Invalid;
	}
}

TilePosition AlliedSquad::getTargetPosition() const {
	// Get common target for majority of the units
	map<TilePosition, int> positions;
	
	for (size_t i = 0; i < mUnits.size(); ++i) {
		TilePosition targetPosition = TilePosition(mUnits[i]->getTargetPosition());
		if (targetPosition != TilePositions::Invalid) {
			positions[targetPosition]++;
		}
	}

	// Return the position which most unit use
	int cMaxUnits = 0;
	TilePosition mostTargetedPosition = TilePositions::Invalid;

	map<TilePosition, int>::const_iterator positionIt;
	for (positionIt = positions.begin(); positionIt != positions.end(); ++positionIt) {
		if (positionIt->second > cMaxUnits) {
			cMaxUnits = positionIt->second;
			mostTargetedPosition = positionIt->first;
		}
	}

	return mostTargetedPosition;
}

bool AlliedSquad::belongsToThisSquad(BWAPI::Unit* pUnit) const {
	for (size_t i = 0; i < mUnits.size(); ++i) {
		if (mUnits[i] == pUnit) {
			return true;
		}
	}

	return false;
}