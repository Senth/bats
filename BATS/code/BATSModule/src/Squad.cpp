#include "Squad.h"
#include "Utilities/Helper.h"
#include "BTHAIModule/Source/UnitAgent.h"
#include "UnitComposition.h"
#include "SquadManager.h"
#include "GameTime.h"
#include "Config.h"
#include "Helper.h"
#include <algorithm>
#include <cmath>

using namespace bats;
using namespace std;
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;
using namespace BWAPI;

int bats::Squad::mcsInstance = 0;
utilities::KeyHandler<_SquadType>* bats::Squad::mpsKeyHandler = NULL;
SquadManager* bats::Squad::mpsSquadManager = NULL;

const int MAX_KEYS = 100;

Squad::Squad(
	const std::vector<UnitAgent*>& units,
	bool avoidEnemyUnits,
	bool disbandable,
	const UnitComposition& unitComposition) 
	:
	mUnitComposition(unitComposition),
	mDisbandable(disbandable),
	mDisbanded(false),
	mTravelsByAir(false),
	mAvoidEnemyUnits(avoidEnemyUnits),
	mId(SquadId::INVALID_KEY),
	mTempGoalPosition(TilePositions::Invalid),
	mGoalPosition(TilePositions::Invalid),
	mRegroupPosition(TilePositions::Invalid),
	mFurthestUnitAwayDistance(0.0),
	mFurthestUnitAwayLastTime(-config::squad::CALC_FURTHEST_AWAY_TIME),
	mGoalState(GoalState_Lim),
	mState(State_Inactive)
{
	// Generate new key for the squad
	if (mcsInstance == 0) {
		utilities::KeyHandler<_SquadType>::init(MAX_KEYS);
		mpsKeyHandler = utilities::KeyHandler<_SquadType>::getInstance();
	}
	mcsInstance++;

	if (NULL == mpsSquadManager) {
		mpsSquadManager = SquadManager::getInstance();
	}

	mId = mpsKeyHandler->allocateKey();

	// Add all units
	addUnits(units);

	if (mUnitComposition.isValid() && !mUnitComposition.isFull()) {
		DEBUG_MESSAGE(utilities::LogLevel_Warning, "Squad::Squad() | Created a Squad with a " <<
			"UnitComposition, but not enough units were supplied for the squad");
	}

	// Add to squad manager
	shared_ptr<Squad> strongPtr = shared_ptr<Squad>(this);
	mThis = weak_ptr<Squad>(strongPtr);
	mpsSquadManager->addSquad(strongPtr);
}

Squad::~Squad() {
	mcsInstance--;

	// Free key
	mpsKeyHandler->freeKey(mId);

	if (mcsInstance == 0) {
		SAFE_DELETE(mpsKeyHandler);
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

void Squad::computeActions() {
	/// @todo avoid enemy units
	if (mAvoidEnemyUnits) {

	}

	switch (mState) {
	case State_Initializing:
		// Only try to create a goal if the player hasn't specified one
		if (mGoalPosition == TilePositions::Invalid &&
			(!mUnitComposition.isValid() ||
			mUnitComposition.isFull()))
		{
			bool goalCreated = createGoal();
			if (goalCreated) {
				mState = State_Active;
			}
		} else {
			mState = State_Active;
		}

		// Update unit movement if we changed to active state
		if (State_Active == mState) {
			updateUnitMovement();
		}
		break;

	case State_Active: {
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


		// Use next via path if close to
		if (!mViaPath.empty() && isCloseTo(mViaPath.front())) {
			mViaPath.pop_front();
			updateUnitMovement();
		}


		// Squad specific and goals
		computeSquadSpecificActions();
		mGoalState = checkGoalState();

		switch (mGoalState) {
		case GoalState_Succeeded:
			onGoalSucceeded();
			break;

		case GoalState_Failed:
			onGoalFailed();
			break;

		default:
			break;
		}

		handleRegroup();
	}
	
	default:
		// Do nothing
		break;
	}
}

BWAPI::TilePosition Squad::getCenter() const {
	if (mUnits.empty()) {
		return BWAPI::TilePositions::Invalid;
	}

	BWAPI::TilePosition center(0,0);
	for (size_t i = 0; i < mUnits.size(); ++i) {
		center += mUnits[i]->getUnit()->getTilePosition();
	}
	center.x() /= mUnits.size();
	center.y() /= mUnits.size();

	return center;
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
	// Skip if it never shall travel by air
	if (!mTravelsByAir) {
		return false;
	}

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

void Squad::addUnit(UnitAgent* pUnit) {
	// Check so that the unit agent doesn't have a squad already.
	assert(pUnit->getSquadId() == SquadId::INVALID_KEY);

	bool bAddUnit = false;

	// Only add units if we have place for them in the unit composition
	if (mUnitComposition.isValid()) {
		bAddUnit = mUnitComposition.addUnit(pUnit);
	}
	// No unit composition, all units can be added
	else {
		bAddUnit = true;
	}

	if (bAddUnit) {	
		mUnits.push_back(pUnit);
		pUnit->setSquadId(mId);
	
		updateUnitMovement(pUnit);
	}
}

void Squad::addUnits(const vector<UnitAgent*>& units) {
	for (size_t i = 0; i < units.size(); ++i) {
		// Check so that the unit doesn't have a squad already.
		addUnit(units[i]);
	}
}

void Squad::removeUnit(UnitAgent* pUnit) {

	vector<UnitAgent*>::iterator foundUnit = std::find(mUnits.begin(), mUnits.end(), pUnit);
	if (foundUnit != mUnits.end()) {
		// Remove from unit composition
		if (mUnitComposition.isValid()) {
			mUnitComposition.removeUnit(*foundUnit);
		}

		mUnits.erase(foundUnit);
	} else {
		ERROR_MESSAGE(false, "Could not find the unit to remove, id: " << pUnit->getUnitID());
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

const BWAPI::TilePosition& Squad::getGoal() const {
	return mGoalPosition;
}

shared_ptr<Squad> Squad::getThis() const {
	return mThis.lock();
}

bool Squad::isCloseTo(const TilePosition& position) const {
	return isCloseTo(position, config::squad::CLOSE_DISTANCE);
}

bool Squad::isCloseTo(const TilePosition& position, double range) const {
	// Skip when regrouping
	if (isRegrouping()) {
		return false;
	}

	bool bClose = false;

	if (travelsByGround()) {
		double squaredDistance = getSquaredDistance(position, getCenter());
		bClose = range * range <= squaredDistance;
	} else {
		double groundDistance = BWTA::getGroundDistance(getCenter(), position);
		bClose = range <= groundDistance;
	}

	return bClose;
}

void Squad::computeSquadSpecificActions() {
	// Does nothing
}

void Squad::setAirTransportation(bool usesAir) {
	mTravelsByAir = usesAir;
}

void Squad::onGoalFailed() {
	/// @todo use the next goal, if no goals are available either disband or create new goal
	/// which one should be the default?
	forceDisband();
}

void Squad::onGoalSucceeded() {
	/// @todo use the next goal, if no goals are available either disband or create new goal
	/// which one should be the default?
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
		const TilePosition& ourBase = Broodwar->self()->getStartLocation();

		// Free all the units from a squad id.
		for (size_t i = 0; i < mUnits.size(); ++i) {
			mUnits[i]->setSquadId(SquadId::INVALID_KEY);

			/// @todo retreat to a better point than our start location.
			/// Maybe add to a defensive squad?			
			mUnits[i]->setGoal(ourBase);
		}

		mDisbanded = true;
	}
}

void Squad::setGoalPosition(const TilePosition& position) {
	mGoalPosition = position;
	updateUnitMovement();
}

void Squad::setViaPath(const std::list<TilePosition>& positions) {
	mViaPath = positions;
	updateUnitMovement();
}

void Squad::addViaPath(const TilePosition& position) {
	mViaPath.push_back(position);
	updateUnitMovement();
}

void Squad::addViaPath(const std::list<TilePosition>& positions) {
	mViaPath.insert(mViaPath.end(), positions.begin(), positions.end());
	updateUnitMovement();
}

Squad::States Squad::getState() const {
	return mState;
}

const vector<UnitAgent*>& Squad::getUnits() const {
	return mUnits;
}

vector<UnitAgent*>& Squad::getUnits() {
	return mUnits;
}

void Squad::setTemporaryGoalPosition(const TilePosition& temporaryPosition) {
	mTempGoalPosition = temporaryPosition;
	updateUnitMovement();
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
			setRegroupPosition(getCenter());
		}
	}
	// Stop regrouping if we're done
	else {
		if (finishedRegrouping()) {
			clearRegroupPosition();
		} else {
			// If a unit is standing still we need to update the regroup position
			if (isAUnitStill()) {
				setRegroupPosition(getCenter());
			}
		}
	}
}

bool Squad::needsRegrouping() const {

	// Use same calculation both for air and ground
	for (size_t i = 0; i < mUnits.size(); ++i) {
		double squaredDistance = getSquaredDistance(getCenter(), mUnits[i]->getUnit()->getTilePosition());

		if (squaredDistance > config::squad::REGROUP_DISTANCE_BEGIN_SQUARED) {
			return true;
		}
	}

	return false;
}

bool Squad::finishedRegrouping() const {
	// Use same calculation both for air and ground
	for (size_t i = 0; i < mUnits.size(); ++i) {
		double squaredDistance = getSquaredDistance(getCenter(), mUnits[i]->getUnit()->getTilePosition());

		if (squaredDistance > config::squad::REGROUP_DISTANCE_END_SQUARED) {
			return false;
		}
	}

	return true;
}

void Squad::setRegroupPosition(const BWAPI::TilePosition& regorupPosition) {
	mRegroupPosition = regorupPosition;

	updateUnitMovement();
}

void Squad::clearRegroupPosition() {
	mRegroupPosition = TilePositions::Invalid;

	updateUnitMovement();
}

void Squad::updateUnitMovement(UnitAgent* pUnit) {
	if (State_Active == mState) {
		pUnit->setGoal(getPriorityMoveToPosition());
	}
}

void Squad::updateUnitMovement() {
	if (State_Active == mState) {
		TilePosition movePosition = getPriorityMoveToPosition();

		for (size_t i = 0; i < mUnits.size(); ++i) {
			mUnits[i]->setGoal(movePosition);
		}
	}
}

TilePosition Squad::getPriorityMoveToPosition() const {
	TilePosition movePosition = TilePositions::Invalid;

	// Regroup
	if (movePosition == TilePositions::Invalid) {
		movePosition = mRegroupPosition;
	}

	// Temp
	if (movePosition == TilePositions::Invalid) {
		movePosition = mTempGoalPosition;
	}

	/// @todo via position when adding unit
	if (movePosition == TilePositions::Invalid && !mViaPath.empty()) {
		movePosition = mViaPath.front();
	}

	// Goal
	if (movePosition == TilePositions::Invalid) {
		movePosition = mGoalPosition;
	}

	return movePosition;
}

bool Squad::isAUnitStill() const {
	for (size_t i = 0; i < mUnits.size(); ++i) {
		Unit* pUnit = mUnits[i]->getUnit();

		if (!pUnit->isMoving() && !pUnit->isAttacking()) {
			return true;
		}
	}

	return false;
}