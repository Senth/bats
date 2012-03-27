#include "Squad.h"
#include "Utilities/Helper.h"
#include "BTHAIModule/Source/UnitAgent.h"
#include "UnitComposition.h"
#include <algorithm>

using namespace bats;
using namespace std;
using std::tr1::shared_ptr;

int bats::Squad::mcsInstance = 0;
utilities::KeyHandler<_SquadType>* bats::Squad::mpsKeyHandler = NULL;

const int MAX_KEYS = 100;

Squad::Squad(
	std::vector<UnitAgent*> units,
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
	mGoalState(GoalState_Lim),
	mState(SquadState_Inactive)
{
	// Generate new key for the squad
	if (mcsInstance == 0) {
		utilities::KeyHandler<_SquadType>::init(MAX_KEYS);
		mpsKeyHandler = utilities::KeyHandler<_SquadType>::getInstance();
	}
	mcsInstance++;

	mId = mpsKeyHandler->allocateKey();

	// Add all units
	addUnits(units);
}

Squad::~Squad() {
	forceDisband();

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

bool Squad::isEmpty() const {
	return mUnits.empty();
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


	switch (mState) {
	case SquadState_Inactive:
		if (!mUnitComposition.isValid() || mUnitComposition.isFull()) {
			bool goalCreated = createGoal();
			if (goalCreated) {
				mState = SquadState_Active;
			}
		}
		break;

	case SquadState_Active:
		switch (mGoalState) {
		case GoalState_Success:
			onGoalSucceeded();
			break;

		case GoalState_Failed:
			onGoalFailed();
			break;

		case GoalState_NotCompleted:
			computeSquadSpecificActions();
			break;

		default:
			break;
		}

		/// @todo implement regroup functionality to the squads.

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
	mUnits.push_back(pUnit);
	pUnit->setSquadId(mId);
	
	// Update goal position if we have one
	if (!mGoalPositions.empty()) {
		pUnit->setGoal(mGoalPositions.front());
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

const SquadId & Squad::getSquadId() const {
	return mId;
}

const BWAPI::TilePosition& Squad::getGoal() const {
	if (!mGoalPositions.empty()) {
		return mGoalPositions.front();
	} else {
		return BWAPI::TilePositions::Invalid;
	}
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
		// Free all the units from a squad id.
		for (size_t i = 0; i < mUnits.size(); ++i) {
			mUnits[i]->setSquadId(SquadId::INVALID_KEY);

			///@todo maybe make the units retreat to a point?
		}

		mDisbanded = true;
	}
}

void Squad::setGoalPosition(const BWAPI::TilePosition& position) {
	mGoalPositions.clear();
	mGoalPositions.push_back(position);
	updateUnitGoals();
}

void Squad::setGoalPositions(const std::list<BWAPI::TilePosition>& positions) {
	mGoalPositions = positions;
	updateUnitGoals();
}

void Squad::addGoalPosition(const BWAPI::TilePosition& position) {
	mGoalPositions.push_back(position);
	updateUnitGoals();
}

void Squad::addGoalPositions(const std::list<BWAPI::TilePosition>& positions) {
	mGoalPositions.insert(mGoalPositions.end(), positions.begin(), positions.end());
	updateUnitGoals();
}

void Squad::updateUnitGoals() {
	BWAPI::TilePosition newGoal = BWAPI::TilePositions::Invalid;
	
	if (!mGoalPositions.empty()) {
		newGoal = mGoalPositions.front();
	}

	for (size_t i = 0; i < mUnits.size(); ++i) {
		mUnits[i]->setGoal(newGoal);
	}
}