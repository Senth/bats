#include "Squad.h"
#include "Utilities/Helper.h"
#include "BTHAIModule/Source/UnitAgent.h"
#include "UnitComposition.h"
#include <algorithm>

using namespace bats;

int bats::Squad::mcsInstance = 0;
utilities::KeyHandler<_SquadType>* bats::Squad::mpsKeyHandler = NULL;

const int MAX_KEYS = 100;

Squad::Squad(
	std::vector<UnitAgent*> units,
	bool disbandable,
	const UnitComposition& unitComposition) 
	:
	mUnits(units),
	mUnitComposition(unitComposition),
	mDisbandable(disbandable),
	mDisbanded(false),
	mId(SquadId::INVALID_KEY)
{
	// Generate new key for the squad
	if (mcsInstance == 0) {
		utilities::KeyHandler<_SquadType>::init(MAX_KEYS);
		mpsKeyHandler = utilities::KeyHandler<_SquadType>::getInstance();
	}
	mcsInstance++;

	mId = mpsKeyHandler->allocateKey();
}

Squad::~Squad() {
	forceDisband();

	mcsInstance--;

	// Free key
	mpsKeyHandler->freeKey(mId);

	if (mcsInstance == 0) {
		delete utilities::KeyHandler<Squad>::getInstance();
	}
}

bool Squad::isDisbanded() const {
	return mDisbanded;
}

bool Squad::isDisbandable() const {
	return mDisbandable;
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
	switch (mState) {
	case SquadState_Inactive:
		if (!mUnitComposition.isValid() || mUnitComposition.isFull()) {
			createGoal();
			mState = SquadState_Active;
		}
		break;

	case SquadState_Active:
		switch (getGoalState()) {
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

bool Squad::isFull() const {
	if (mUnitComposition.isValid()) {
		return mUnitComposition.isFull();
	} else {
		return false;
	}
}

void Squad::addUnit(UnitAgent* pUnit) {
	// Check so that the unit agent doesn't have a squad already.
	assert(pUnit->getSquadId() == SquadId::INVALID_KEY);
	mUnits.push_back(pUnit);
}

void Squad::addUnits(const std::vector<UnitAgent*>& units) {
	for (size_t i = 0; i < units.size(); ++i) {
		// Check so that the unit doesn't have a squad already.
		assert(units[i]->getSquadId() == SquadId::INVALID_KEY);
		mUnits.push_back(units[i]);
	}
}

void Squad::removeUnit(UnitAgent* pUnit) {
	std::vector<UnitAgent*>::iterator foundUnit = std::find(mUnits.begin(), mUnits.end(), pUnit);
	if (foundUnit != mUnits.end()) {
		mUnits.erase(foundUnit);
	} else {
		ERROR_MESSAGE(false, "Could not find the unit to remove, id: " << pUnit->getUnitID());
	}
}

const SquadId & Squad::getSquadId() const {
	return mId;
}

void Squad::computeSquadSpecificActions() {
	// Does nothing
}

void Squad::onGoalFailed() {
	// Disbands the squad, even if it's not disbandable.
	forceDisband();
}

void Squad::onGoalSucceeded() {
	// Disbands the squad, even if it's not disbandable.
	forceDisband();
}

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
}

void Squad::setGoalPositions(const std::list<BWAPI::TilePosition>& positions) {
	mGoalPositions = positions;
}

void Squad::addGoalPosition(const BWAPI::TilePosition& position) {
	mGoalPositions.push_back(position);
}

void Squad::addGoalPositions(const std::list<BWAPI::TilePosition>& positions) {
	mGoalPositions.insert(mGoalPositions.end(), positions.begin(), positions.end());
}