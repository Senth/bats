#include "AlliedSquad.h"
#include "Helper.h"

using namespace bats;

utilities::KeyHandler<_AlliedSquadType>* AlliedSquad::mpsKeyHandler = NULL;
int AlliedSquad::mcsInstances = 0;

const int MAX_KEYS = 1000;

AlliedSquad::AlliedSquad(bool big) : mId(AlliedSquadId::INVALID_KEY) {
	mBig = big;
	mState = State_Idle;

	if (mcsInstances == 0) {
		utilities::KeyHandler<_AlliedSquadType>::init(MAX_KEYS);
		mpsKeyHandler = utilities::KeyHandler<_AlliedSquadType>::getInstance();
	}
	mcsInstances++;

	mId = mpsKeyHandler->allocateKey();

	// Add listener
	config::addOnConstantChangedListener("classification", "squad", "measure_time", this);
}

AlliedSquad::~AlliedSquad() {
	// Remove listener
	config::removeOnConstantChangedListener("classification", "squad", "measure_time", this);
	
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

void AlliedSquad::onConstantChanged(const std::string& section, const std::string& subsection, const std::string& variable) {
	// measure_time
	if (section == "classification" &&
		subsection == "squad" &&
		variable == "measure_time")
	{
		// If less then erase those at the back
		if (config::classification::squad::MEASURE_TIME < mCenter.size()) {
			mCenter.resize(config::classification::squad::MEASURE_TIME);
		}
	}
}

AlliedSquad::States AlliedSquad::getState() const {
	return mState;
}

void AlliedSquad::update() {
	/// @todo

	// Retreating
	if (isRetreating()) {

	}
	// Attacking
	else if (isAttacking()) {

	}
	// Moving to attack
	else if (isMovingToAttack()) {

	}
	// Stopped moving to attack
	else if (hasHaltedAttack()) {

	}
	// else - Idle
	else {
		mState = State_Idle;
	}
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
	/// @todo
	return false;
}

bool AlliedSquad::isRetreating() const {
	/// @todo
	return false;
}

bool AlliedSquad::isAttacking() const {
	/// @todo
	return false;
}

bool AlliedSquad::hasHaltedAttack() const {
	/// @todo
	return false;
}