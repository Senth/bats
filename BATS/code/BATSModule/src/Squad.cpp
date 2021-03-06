#include "Squad.h"
#include "UnitComposition.h"
#include "SquadManager.h"
#include "IntentionWriter.h"
#include "GameTime.h"
#include "Config.h"
#include "Helper.h"
#include "BTHAIModule/Source/UnitAgent.h"
#include "BTHAIModule/Source/WorkerAgent.h"
#include "Utilities/Helper.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <BWTA.h>

using namespace bats;
using namespace std;
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;
using namespace BWAPI;

int bats::Squad::mscInstances = 0;
utilities::KeyHandler<_SquadType>* bats::Squad::msKeyHandler = NULL;
SquadManager* bats::Squad::msSquadManager = NULL;
GameTime* bats::Squad::msGameTime = NULL;
IntentionWriter* bats::Squad::msIntentionWriter = NULL;

const int MAX_KEYS = 100;
const double FROM_TILE_TO_POSITION = 32.0;
const double FROM_POSITION_TO_TILE = 1.0 / FROM_TILE_TO_POSITION;
const int UNIT_SMALL_SIZE = 1;
const int UNIT_MEDIUM_SIZE = UNIT_SMALL_SIZE * 2;
const int UNIT_LARGE_SIZE = UNIT_SMALL_SIZE * 4;

Squad::Squad(
	const std::vector<UnitAgent*>& units,
	bool avoidEnemyUnits,
	bool disbandable,
	const UnitComposition& unitComposition) 
	:
	mUnitComposition(unitComposition),
	mDisbandable(disbandable),
	mDisbanded(false),
	mAvoidEnemyUnits(avoidEnemyUnits),
	mId(SquadId::INVALID_KEY),
	mTempGoalPosition(TilePositions::Invalid),
	mGoalPosition(TilePositions::Invalid),
	mRegroupPosition(TilePositions::Invalid),
	mRetreatPosition(TilePositions::Invalid),
	mFurthestUnitAwayDistance(0.0),
	mFurthestUnitAwayLastTime(-config::squad::CALC_FURTHEST_AWAY_TIME),
	mGoalState(GoalState_Lim),
	mState(State_Initializing),
	mCanRegroup(true),
	mUpdateLast(0.0),
	mInitialized(false),
	mcMechanicalUnits(0),
	mcOrganicUnits(0),
	mcWorkers(0)
{
	// Generate new key for the squad
	if (mscInstances == 0) {
		utilities::KeyHandler<_SquadType>::init(MAX_KEYS);
		msKeyHandler = utilities::KeyHandler<_SquadType>::getInstance();
	}
	mscInstances++;

	if (NULL == msSquadManager) {
		msSquadManager = SquadManager::getInstance();
		msGameTime = GameTime::getInstance();
		msIntentionWriter = IntentionWriter::getInstance();
	}

	mId = msKeyHandler->allocateKey();

	// Add all units
	addUnits(units);

	if (mUnitComposition.isValid() && !mUnitComposition.isFull()) {
		DEBUG_MESSAGE(utilities::LogLevel_Warning, "Squad::Squad() | Created a Squad with a " <<
			"UnitComposition, but not enough units were supplied for the squad");
	}

	// Add to squad manager
	SquadPtr strongPtr(this);
	mThis = weak_ptr<Squad>(strongPtr);
	msSquadManager->addSquad(strongPtr);

	// Add listener
	config::addOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE), this);
}

Squad::~Squad() {
	// Remove listener
	config::removeOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE), this);

	mscInstances--;

	// Free key
	msKeyHandler->freeKey(mId);

	if (mscInstances == 0) {
		msIntentionWriter = NULL;
		msGameTime = NULL;
		msSquadManager = NULL;

		SAFE_DELETE(msKeyHandler);
	}
}

bool Squad::isDisbanded() const {
	return mDisbanded;
}

bool Squad::isDisbandable() const {
	return mDisbandable;
}

bool Squad::isRegrouping() const {
	return mRegroupPosition != TilePositions::Invalid;
}

bool Squad::isEmpty() const {
	return mUnits.empty();
}

void Squad::deactivate() {
	mState = State_Inactive;
}

void Squad::activate() {
	mState = State_Initializing;
}

bool Squad::tryDisband() {
	if (mDisbandable) {
		forceDisband();
		return true;
	} else {
		return false;
	}
}

void Squad::update() {
	// Don't call to often
	if (msGameTime->getElapsedTime() - mUpdateLast < config::classification::squad::MEASURE_INTERVAL_TIME) {
		return;
	}
	mUpdateLast = msGameTime->getElapsedTime();

	updateSupply();


	// Check if this is the first time calling, then add all the initial units
	if (!mInitialized) {
		mInitialized = true;
		for (size_t i = 0; i < mUnits.size(); ++i) {
			onUnitAdded(mUnits[i]);
		}
	}

	/// @todo Remove states in a future version
	switch (mState) {
	case State_Initializing:
		// Player set a goal directly, active the squad
		if (mGoalPosition != TilePositions::Invalid) {
			mState = State_Active;
		}
		// Else create a goal
		else if (!mUnitComposition.isValid() || mUnitComposition.isFull()) {
			createGoal();
			mState = State_Active;
		}

		// Update unit movement if we changed to active state
		if (State_Active == mState) {
			updateUnitMovement();
		}
		break;

	case State_Active: {
		handleWaitGoals();

		// Use next via path if close to
		if (!mViaPath.empty() && isCloseTo(mViaPath.front())) {
			mViaPath.pop_front();
			updateUnitMovement();
		}


		// Squad specific and goals
		updateDerived();


		// Handle goals and retreats if we're not retreating
		if (!isRetreating()) {
			handleGoal();
			handleRegroup();
		} else {
			handleRetreat();
		}
	}
	
	default:
		// Do nothing
		break;
	}
}

void Squad::handleRetreat() {
	if (mRetreatPosition != TilePositions::Invalid) {
		if (isCloseTo(mRetreatPosition)) {
			mRetreatPosition = TilePositions::Invalid;
			onRetreatCompleted();
			// Shall update unit movement really be called?
			updateUnitMovement();
		}
	}
}

void Squad::handleGoal() {
	mGoalState = checkGoalState();

	switch (mGoalState) {
	case GoalState_Succeeded:
		DEBUG_MESSAGE(utilities::LogLevel_Fine, getName() << ": Successfully completed goal");
		onGoalSucceeded();
		break;

	case GoalState_Failed:
		DEBUG_MESSAGE(utilities::LogLevel_Fine, getName() << ": Failed current goal");
		onGoalFailed();
		break;

	default:
		break;
	}
}

void Squad::handleWaitGoals() {
	// Remove finished WaitGoals and call onWaitGoalFinished for finished events.
	vector<shared_ptr<WaitGoal>>::iterator waitGoalIt;
	waitGoalIt = mWaitGoals.begin();
	while (mWaitGoals.end() != waitGoalIt) {
		if ((*waitGoalIt)->getWaitState() != WaitState_Waiting) {
			shared_ptr<WaitGoal> finishedWaitGoal = *waitGoalIt;
			waitGoalIt = mWaitGoals.erase(waitGoalIt);
			onWaitGoalFinished(finishedWaitGoal);
		} else {
			++waitGoalIt;
		}
	}
}

BWAPI::TilePosition Squad::getCenter() const {
	if (mUnits.empty()) {
		return BWAPI::TilePositions::Invalid;
	}

	BWAPI::TilePosition center(0,0);
	int cUnits = 0;
	for (size_t i = 0; i < mUnits.size(); ++i) {

		// Only use center position of units that aren't loaded
		if (!mUnits[i]->getUnit()->isLoaded()) {
			++cUnits;
			center += mUnits[i]->getUnit()->getTilePosition();
		}
	}

	if (cUnits > 0) {
		center.x() /= cUnits;
		center.y() /= cUnits;
	} else {
		center = TilePositions::Invalid;
	}

	return center;
}

void Squad::setCanRegroup(bool canRegroup) {
	mCanRegroup = canRegroup;
}

double Squad::getFurthestUnitAwayDistance(bool forceRecalculate) const {
	double furthestAway = 0.0;

	double currentGameTime = GameTime::getInstance()->getElapsedTime();
	if (forceRecalculate ||
		currentGameTime - mFurthestUnitAwayLastTime >= config::squad::CALC_FURTHEST_AWAY_TIME)
	{
		mFurthestUnitAwayLastTime = currentGameTime;

		const BWAPI::TilePosition& center = getCenter();

		for (size_t i = 0; i < mUnits.size(); ++i) {
			double squaredDistance = getSquaredDistance(center, mUnits[i]->getUnit()->getTilePosition());

			if (squaredDistance > furthestAway) {
				furthestAway = squaredDistance;
			}
		}

		// return actual distance and not squared
		if (furthestAway > 0.0) {
			furthestAway = sqrt(furthestAway);
		}
	}

	return furthestAway;
}

bool Squad::isFull() const {
	if (mUnitComposition.isValid()) {
		return mUnitComposition.isFull();
	} else {
		return false;
	}
}

bool Squad::travelsByGround() const {
	return !travelsByAir();
}

bool Squad::travelsByAir() const {
	// Calculate if it can travel by air.
	int groundSlotsRequired = 0;
	int transportationSlots = 0;
	for (size_t i = 0; i < mUnits.size(); ++i) {
		if (mUnits[i]->isGround()) {
			groundSlotsRequired += mUnits[i]->getUnitType().spaceRequired();
		}
		// Note, it must be air since it's not a ground unit. No need to check that
		else if (mUnits[i]->isTransport()) {
			transportationSlots += mUnits[i]->getUnitType().spaceProvided();
		}
	}

	if (groundSlotsRequired <= transportationSlots) {
		return true;
	} else {
		return false;
	}
}

void Squad::addUnit(UnitAgent* unit) {
	bool bAddUnit = false;

	// Only add units if we have place for them in the unit composition
	if (mUnitComposition.isValid()) {
		bAddUnit = mUnitComposition.addUnit(unit);
	}
	// No unit composition, all units can be added
	else {
		bAddUnit = true;
	}

	if (bAddUnit) {	
		// Remove unit from existing squad if it belongs to another squad.
		if (unit->getSquadId() != SquadId::INVALID_KEY) {
			SquadPtr pOldSquad = msSquadManager->getSquad(unit->getSquadId());
			assert(pOldSquad != NULL);

			pOldSquad->removeUnit(unit);
		}


		// Add unit
		mUnits.push_back(unit);
		unit->setSquadId(mId);
	
		updateUnitMovement(unit);

		// Set has ground or has air
		if (unit->isAir()) {
			mHasAirUnits = true;
		} else {
			mHasGroundUnits = true;
		}

		// Set mechanical or biological
		if (unit->getUnitType().isMechanical()) {
			mcMechanicalUnits++;
		} else if (unit->getUnitType().isOrganic()) {
			mcOrganicUnits++;
		}

		if (unit->getUnitType().isWorker()) {
			mcWorkers++;
		}

		// Send event only if the class has been initialized (otherwise derived classes doesn't exist)
		if (mInitialized) {
			onUnitAdded(unit);
		}
	}
}

void Squad::addUnits(const vector<UnitAgent*>& units) {
	for (size_t i = 0; i < units.size(); ++i) {
		// Check so that the unit doesn't have a squad already.
		addUnit(units[i]);
	}
}

void Squad::removeUnit(UnitAgent* unit) {

	vector<UnitAgent*>::iterator foundUnit = std::find(mUnits.begin(), mUnits.end(), unit);
	if (foundUnit != mUnits.end()) {
		// Remove from unit composition
		if (mUnitComposition.isValid()) {
			mUnitComposition.removeUnit(*foundUnit);
		}

		mUnits.erase(foundUnit);

		// Update has air and has ground
		if (unit->isAir()) {
			mHasAirUnits = false;

			// Look for air units
			size_t i = 0;
			while (i < mUnits.size() && !mHasAirUnits) {
				if (mUnits[i]->isAir()) {
					mHasAirUnits = true;
				}
				++i;
			}
		} else {
			mHasGroundUnits = false;

			// Look for air units
			size_t i = 0;
			while (i < mUnits.size() && !mHasGroundUnits) {
				if (mUnits[i]->isGround()) {
					mHasGroundUnits = true;
				}
				++i;
			}
		}

		// Update mechanical and biological
		if (unit->getUnitType().isMechanical()) {
			mcMechanicalUnits--;
		} else if (unit->getUnitType().isOrganic()) {
			mcOrganicUnits--;
		}

		if (unit->getUnitType().isWorker()) {
			mcWorkers--;
		}

		unit->setSquadId(bats::SquadId::INVALID_KEY);
		onUnitRemoved(unit);
		unit->resetToDefaultBehavior();

	} else {
		ERROR_MESSAGE(false, "Could not find the unit to remove, id: " << unit->getUnitID());
	}
}

void Squad::addWaitGoal(const shared_ptr<WaitGoal>& waitGoal) {
	assert(NULL != waitGoal);
	mWaitGoals.push_back(waitGoal);
	onWaitGoalAdded(waitGoal);
}

void Squad::addWaitGoals(const vector<shared_ptr<WaitGoal>>& waitGoals) {
	for (size_t i = 0; i < waitGoals.size(); ++i) {
		assert(NULL == waitGoals[i]);
		mWaitGoals.push_back(waitGoals[i]);
		onWaitGoalAdded(waitGoals[i]);
	}
}

bool Squad::hasWaitGoals() const {
	return !mWaitGoals.empty();
}

const SquadId & Squad::getSquadId() const {
	return mId;
}

const BWAPI::TilePosition& Squad::getGoalPosition() const {
	return mGoalPosition;
}

SquadCstPtr Squad::getThis() const {
	return mThis.lock();
}

SquadPtr Squad::getThis() {
	return mThis.lock();
}

bool Squad::isCloseTo(const BWAPI::TilePosition& position) const {
	return isCloseTo(position, config::squad::CLOSE_DISTANCE);
}

bool Squad::isCloseTo(const BWAPI::TilePosition& position, int range) const {
	// Skip when regrouping
	if (isRegrouping()) {
		return false;
	}

	// To be more effective, use squared distance. If we're in range check for ground distance
	// if the squad travels by ground. Ground distance is very computational heavy
	bool bClose = isWithinRange(position, getCenter(), range);

	if (bClose && travelsByGround()) {
		int groundDistance = static_cast<int>(BWTA::getGroundDistance(getCenter(), position) * FROM_POSITION_TO_TILE + 0.5);
		bClose = groundDistance <= range;
	}

	return bClose;
}

bool Squad::isAvoidingEnemies() const {
	return mAvoidEnemyUnits || isRetreating();
}

void Squad::updateDerived() {
	// Does nothing
}

void Squad::onGoalFailed() {
	/// @todo Disband or create new goal, which one should be the default?
	forceDisband();
}

void Squad::onGoalSucceeded() {
	/// @todo Disband or create new goal, which one should be the default?
	forceDisband();
}

bool Squad::isRetreating() const {
	return mRetreatPosition != TilePositions::Invalid;
}

void Squad::setRetreatPosition(const BWAPI::TilePosition& retreatPosition) {
	if (mRetreatPosition != retreatPosition) {
		mRetreatPosition = retreatPosition;
		if (mRetreatPosition != TilePositions::Invalid) {
			setAvoidEnemyUnits(true);
			mRegroupPosition = TilePositions::Invalid;
		}
		updateUnitMovement();
	}
}

void Squad::onRetreatCompleted() {
	forceDisband();
}

#pragma warning(push)
#pragma warning(disable:4100)
void Squad::onWaitGoalAdded(const shared_ptr<WaitGoal>& newWaitGoal) {
	// Does nothing
}

void Squad::onWaitGoalFinished(const shared_ptr<WaitGoal>& finishedWaitGoal) {
	// Does nothing
}
#pragma warning(pop)

void Squad::forceDisband() {
	if (!mDisbanded) {
		// Free all the units from a squad.
		for (size_t i = 0; i < mUnits.size(); ++i) {
			mUnits[i]->setSquadId(SquadId::INVALID_KEY);
			onUnitRemoved(mUnits[i]);
			mUnits[i]->resetToDefaultBehavior();
		}

		mUnits.clear();

		mDisbanded = true;
	}
}

void Squad::setGoalPosition(const TilePosition& position) {
	if (mGoalPosition != position) {
		mGoalPosition = position;
		updateUnitMovement();
	}
}

const TilePosition& Squad::getRetreatPosition() const {
	return mRetreatPosition;
}

void Squad::setViaPath(const std::list<TilePosition>& positions) {
	mViaPath = positions;
	updateUnitMovement();
}

void Squad::addViaPath(const BWAPI::TilePosition& position) {
	mViaPath.push_back(position);
	updateUnitMovement();
}

void Squad::addViaPath(const std::list<BWAPI::TilePosition>& positions) {
	mViaPath.insert(mViaPath.end(), positions.begin(), positions.end());
	updateUnitMovement();
}

Squad::States Squad::getState() const {
	return mState;
}

const vector<const UnitAgent*>& Squad::getUnits() const {
	return *reinterpret_cast<const vector<const UnitAgent*>*>(&mUnits);
}

const vector<UnitAgent*>& Squad::getUnits() {
	return mUnits;
}

void Squad::setTemporaryGoalPosition(const TilePosition& temporaryPosition) {
	if (mTempGoalPosition != temporaryPosition) {
		mTempGoalPosition = temporaryPosition;
		updateUnitMovement();
	}
}

const TilePosition& Squad::getTemporaryGoalPosition() const {
	return mTempGoalPosition;
}

bool Squad::hasTemporaryGoalPosition() const {
	return mTempGoalPosition != TilePositions::Invalid;
}

void Squad::handleRegroup() {
	/// @todo improve regrouping position for ground units. Regroup position shall be between
	/// the two units that are furthest away from each other (ground distance), and not
	/// the center, which sometimes are inaccessible.
	
	// Not regrouping, but needs regrouping
	if (mRegroupPosition == TilePositions::Invalid) {
		if (needsRegrouping()) {
			setRegroupPosition(findRegroupPosition());
		}
	}
	// Stop regrouping if we're done
	else {
		if (finishedRegrouping()) {
			clearRegroupPosition();
		}
		// Regroup "timed out" trying a new position
		else if (msGameTime->getElapsedTime() >= mRegroupStartTime + config::squad::REGROUP_NEW_POSITION_TIME &&
			isAUnitStill())
		{
			setRegroupPosition(findRegroupPosition());
		}
	}
}

bool Squad::needsRegrouping() const {
	// We never need regrouping if the squad has set it to never regroup
	if (!mCanRegroup) {
		return false;
	}

	const TilePosition& center = getCenter();
	int regroupDistanceBegin = config::squad::REGROUP_DISTANCE_BEGIN + getRegroupIncrement();

	// Use same calculation both for air and ground
	for (size_t i = 0; i < mUnits.size(); ++i) {
		if (!isWithinRange(center, mUnits[i]->getUnit()->getTilePosition(), regroupDistanceBegin)) {
			return true;
		}
	}

	return false;
}

bool Squad::finishedRegrouping() const {
	// We have always finished regrouping if the squad can't regroup any more
	if (!mCanRegroup) {
		return true;
	}

	int regroupDistanceEnd = config::squad::REGROUP_DISTANCE_END + getRegroupIncrement();

	// Use same calculation both for air and ground
	for (size_t i = 0; i < mUnits.size(); ++i) {
		if (!isWithinRange(mRegroupPosition, mUnits[i]->getUnit()->getTilePosition(), regroupDistanceEnd)) {
			return false;
		}
	}

	return true;
}

int Squad::getRegroupIncrement() const {
	return static_cast<int>(ceil(config::squad::REGROUP_DISTANCE_INCREMENT * getDimensionSize()));
}

void Squad::setRegroupPosition(const BWAPI::TilePosition& regroupPosition) {
	if (mRegroupPosition != regroupPosition) {
		mRegroupPosition = regroupPosition;
		mRegroupStartTime = msGameTime->getElapsedTime();
		updateUnitMovement();
	}
}

void Squad::clearRegroupPosition() {
	mRegroupPosition = TilePositions::Invalid;
	updateUnitMovement();
}

void Squad::updateUnitMovement(UnitAgent* unit) {
	if (State_Active == mState) {
		unit->setGoal(getPriorityMoveToPosition());
	}
}

void Squad::updateUnitMovement() {
	if (State_Active == mState) {
		TilePosition movePosition = getPriorityMoveToPosition();

		for (size_t i = 0; i < mUnits.size(); ++i) {
			/*if(mUnits[i]->isWorker()){
				WorkerAgent * w = new WorkerAgent(mUnits[i]->getUnit());
				w->setState(WorkerAgent::MOVE_TO_SPOT);
			}*/
			mUnits[i]->setGoal(movePosition);
		}
	}
}

void Squad::setAvoidEnemyUnits(bool bAvoidEnemyUnits) {
	mAvoidEnemyUnits = bAvoidEnemyUnits;
}

void Squad::clearMovement() {
	mViaPath.clear();
	mGoalPosition = TilePositions::Invalid;
	mTempGoalPosition = TilePositions::Invalid;
	mRegroupPosition = TilePositions::Invalid;
	mRetreatPosition = TilePositions::Invalid;
}

TilePosition Squad::getPriorityMoveToPosition() const {
	TilePosition movePosition = TilePositions::Invalid;

	// Regroup
	if (movePosition == TilePositions::Invalid) {
		movePosition = mRegroupPosition;
		
		DEBUG_MESSAGE_CONDITION(
			movePosition != TilePositions::Invalid,
			utilities::LogLevel_Finer,
			getName() << ": New regroup position, " << movePosition
		);
	}

	// Temp, don't use temp while retreating
	if (movePosition == TilePositions::Invalid && !isRetreating()) {
		movePosition = mTempGoalPosition;
		
		DEBUG_MESSAGE_CONDITION(
			movePosition != TilePositions::Invalid,
			utilities::LogLevel_Finer,
			getName() << ": New temp position, " << movePosition
		);
	}

	// Via
	if (movePosition == TilePositions::Invalid && !mViaPath.empty()) {
		movePosition = mViaPath.front();
		
		DEBUG_MESSAGE_CONDITION(
			movePosition != TilePositions::Invalid,
			utilities::LogLevel_Finer,
			getName() << ": New via position, " << movePosition
		);
	}

	// Retreat
	if (movePosition == TilePositions::Invalid) {
		movePosition = mRetreatPosition;

		DEBUG_MESSAGE_CONDITION(
			movePosition != TilePositions::Invalid,
			utilities::LogLevel_Finer,
			getName() << ": New retreat position, " << movePosition
		);
	}

	// Goal
	if (movePosition == TilePositions::Invalid) {
		movePosition = mGoalPosition;
		
		DEBUG_MESSAGE_CONDITION(
			movePosition != TilePositions::Invalid,
			utilities::LogLevel_Finer,
			getName() << ": New goal position, " << movePosition
		);
	}

	return movePosition;
}

bool Squad::isAUnitStill() const {
	for (size_t i = 0; i < mUnits.size(); ++i) {
		Unit* unit = mUnits[i]->getUnit();

		if (!unit->isMoving() && !unit->isAttacking()) {
			return true;
		}
	}

	return false;
}

bool Squad::hasAir() const {
	return mHasAirUnits;
}

bool Squad::hasGround() const {
	return mHasGroundUnits;
}

bool Squad::isEnemyAttackUnitsWithinSight() const {
	// Skip if we're regrouping
	if (isRegrouping()) {
		return false;
	}

	// Use double squared distance (squared for faster calculation)
	int sightDistance = getSightDistance();

	Position center = Position(getCenter());

	// Check for nearby enemies, RETURN early
	const std::set<Unit*>& units = Broodwar->getAllUnits();	
	std::set<Unit*>::const_iterator unitIt;
	for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
		// Only process enemy units that can attack
		if (Broodwar->self()->isEnemy((*unitIt)->getPlayer()) && (*unitIt)->getType().canAttack()) {
			
			bool canAttackUs = false;

			// If we only have ground units, skip units that cannot attack ground
			if (hasGround() && !hasAir()) {
				if ((*unitIt)->getType().groundWeapon() != WeaponTypes::None) {
					canAttackUs = true;
				}
			}
			// We only have air units, skip units that only cannot attack air
			else if (hasAir() && !hasGround()) {
				if ((*unitIt)->getType().airWeapon() != WeaponTypes::None) {
					canAttackUs = true;
				}
			}
			// Both ground and air, all attacking units count
			else {
				canAttackUs = true;
			}

			// Check if the unit is within range
			if (canAttackUs) {
				if (bats::isWithinRange(center, (*unitIt)->getPosition(), sightDistance)) {
					return true;
				}
			}
		}
	}

	return false;
}

vector<Unit*> Squad::getEnemyUnitsWithinSight(bool onlyAttackingUnits) const {

	int sightDistanceSquared = getSightDistance();
	sightDistanceSquared *= sightDistanceSquared;
	Position center = Position(getCenter());

	std::vector<Unit*> foundUnits;
	const std::set<Unit*>& units = Broodwar->getAllUnits();	
	std::set<Unit*>::const_iterator unitIt;
	for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
		// Only process enemy units
		if (Broodwar->self()->isEnemy((*unitIt)->getPlayer())) {

			// If we only want attacking units
			if (!onlyAttackingUnits || (*unitIt)->getType().canAttack()) {

				// Check if the unit is within range
				int diffDistance = getSquaredDistance(center, (*unitIt)->getPosition());
				if (diffDistance <= sightDistanceSquared) {
					foundUnits.push_back(*unitIt);
				}
			}
		}
	}

	return foundUnits;
}

int Squad::getSightDistance() const {
	int maxSight = 0;
	for (size_t i = 0; i < mUnits.size(); ++i) {
		int unitSightCurrent = Broodwar->self()->sightRange(mUnits[i]->getUnitType());
		if (unitSightCurrent > maxSight) {
			maxSight = unitSightCurrent;
		}
	}

	return static_cast<int>(maxSight * config::squad::SIGHT_DISTANCE_MULTIPLIER);
}

void Squad::printGraphicDebugInfo() const {
	// Low
	// Print type, id, number of units, number of supplies of squad
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Low) {
		const string& info = getDebugString();

		const Position squadCenter = Position(getCenter());

		BWAPI::Broodwar->drawTextMap(squadCenter.x(), squadCenter.y(), "%s", info.c_str());
	}


	// High
	// Print regroup distances for the squad
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_High) {

		if (mCanRegroup) {
			// Draw regroup position and perimeter for when the regrouping is done
			if (mRegroupPosition != TilePositions::Invalid) {
				const Position regroupPosition = Position(mRegroupPosition);

				BWAPI::Broodwar->drawCircleMap(
					regroupPosition.x(),
					regroupPosition.y(),
					config::squad::REGROUP_DISTANCE_END + getRegroupIncrement() * TILE_SIZE,
					Colors::Orange
				);
			}
			// Draw the begin regroup perimeter for the squad
			else {
				const Position squadCenter = Position(getCenter());

				BWAPI::Broodwar->drawCircleMap(
					squadCenter.x(),
					squadCenter.y(),
					config::squad::REGROUP_DISTANCE_BEGIN + getRegroupIncrement() * TILE_SIZE,
					Colors::Green
				);
			}
		}
	}
}

string Squad::getDebugString() const {
	stringstream ss;
	ss << TextColors::ROYAL_BLUE << left <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Id: " << mId << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Type: " << getName() << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Units: " << getUnitCount() << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Supplies: " << getSupplyCount() << "\n";

	return ss.str();
}

size_t Squad::getUnitCount() const {
	return mUnits.size();
}

int Squad::getSupplyCount() const {
	int cSupply = 0;
	for (size_t i = 0; i < mUnits.size(); ++i) {
		cSupply += mUnits[i]->getUnitType().supplyRequired();
	}

	return cSupply;
}

int Squad::getDeltaSupplyCount() const {
	if (mSupplies.size() == config::classification::squad::MEASURE_SIZE) {
		return mSupplies.front() - mSupplies.back();
	} else {
		return 0;
	}
}

const UnitComposition& Squad::getUnitComposition() const {
	return mUnitComposition;
}

void Squad::onConstantChanged(config::ConstantName constantName) {
	if (constantName == TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE)) {
		// If less then erase those at the back
		if (config::classification::squad::MEASURE_SIZE < mSupplies.size()) {
			mSupplies.resize(config::classification::squad::MEASURE_SIZE);
		}
	}
}

void Squad::updateSupply() {
	mSupplies.push_front(getSupplyCount());

	if (mSupplies.size() > config::classification::squad::MEASURE_SIZE) {
		mSupplies.pop_back();
	}
}

int Squad::getDimensionSize() const {
	int size = 0;
	for (size_t i = 0; i < mUnits.size(); ++i) {
		const UnitSizeType& unitSizeType = mUnits[i]->getUnitType().size();

		if (unitSizeType == UnitSizeTypes::Small) {
			size += UNIT_SMALL_SIZE;
		} else if (unitSizeType == UnitSizeTypes::Medium) {
			size += UNIT_MEDIUM_SIZE;
		} else if (unitSizeType == UnitSizeTypes::Large) {
			size += UNIT_LARGE_SIZE;
		}
	}

	return size;
}

BWAPI::TilePosition Squad::findRegroupPosition() const {
	if (mGoalPosition == TilePositions::Invalid) {
		return TilePositions::Invalid;
	}
	
	BWAPI::TilePosition bestRegroup = TilePositions::Invalid;
	int bestDist = INT_MAX;

	for (size_t i = 0; i < mUnits.size(); ++i) {
		const TilePosition& unitPos = mUnits[i]->getUnit()->getTilePosition();
		int dist = bats::getSquaredDistance(unitPos, mGoalPosition);
		if (dist < bestDist) {
			bestDist = dist;
			bestRegroup = unitPos;
		}
	}

	return bestRegroup;
}

bool Squad::hasMechanicalUnits() const {
	return mcMechanicalUnits > 0;
}

bool Squad::hasOrganicUnits() const {
	return mcOrganicUnits > 0;
}

size_t Squad::getMechanicalUnitCount() const {
	return mcMechanicalUnits;
}

size_t Squad::getOrganicUnitCount() const {
	return mcOrganicUnits;
}

size_t Squad::getWorkerCount() const {
	return mcWorkers;
}