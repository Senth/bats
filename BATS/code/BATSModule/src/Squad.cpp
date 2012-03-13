#include "Squad.h"
#include "Utilities/Helper.h"
#include "BTHAIModule/Source/UnitAgent.h"
#include "UnitComposition.h"

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

void Squad::moveTo(const BWAPI::TilePosition& goalPosition, const std::vector<BWAPI::TilePosition>& via) {
	mGoalPosition = goalPosition;

	if (!via.empty()) {
		mMoveVia = via;
	}
}