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
#include <sstream>
#include <iomanip>

#include "BTHAIModule/Source/WorkerAgent.h"

using namespace bats;
using namespace std;
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;
using namespace BWAPI;

int bats::Squad::mcsInstance = 0;
utilities::KeyHandler<_SquadType>* bats::Squad::mpsKeyHandler = NULL;
SquadManager* bats::Squad::mpsSquadManager = NULL;
GameTime* bats::Squad::mpsGameTime = NULL;

const int MAX_KEYS = 100;
const double FROM_TILE_TO_POSITION = 32.0;
const double FROM_POSITION_TO_TILE = 1.0 / FROM_TILE_TO_POSITION;

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
	mInitialized(false)
{
	// Generate new key for the squad
	if (mcsInstance == 0) {
		utilities::KeyHandler<_SquadType>::init(MAX_KEYS);
		mpsKeyHandler = utilities::KeyHandler<_SquadType>::getInstance();
	}
	mcsInstance++;

	if (NULL == mpsSquadManager) {
		mpsSquadManager = SquadManager::getInstance();
		mpsGameTime = GameTime::getInstance();
	}

	mId = mpsKeyHandler->allocateKey();

	// Add all units
	addUnits(units);

	if (mUnitComposition.isValid() && !mUnitComposition.isFull()) {
		DEBUG_MESSAGE(utilities::LogLevel_Warning, "Squad::Squad() | Created a Squad with a " <<
			"UnitComposition, but not enough units were supplied for the squad");
	}

	// Add to squad manager
	SquadPtr strongPtr(this);
	mThis = weak_ptr<Squad>(strongPtr);
	mpsSquadManager->addSquad(strongPtr);

	// Add listener
	config::addOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE), this);
}

Squad::~Squad() {
	// Remove listener
	config::removeOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE), this);

	mcsInstance--;

	// Free key
	mpsKeyHandler->freeKey(mId);

	if (mcsInstance == 0) {
		SAFE_DELETE(mpsKeyHandler);
		mpsGameTime = NULL;
		mpsSquadManager = NULL;
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
	if (mpsGameTime->getElapsedTime() - mUpdateLast < config::classification::squad::MEASURE_INTERVAL_TIME) {
		return;
	}
	mUpdateLast = mpsGameTime->getElapsedTime();


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
			bool goalCreated = createGoal();
			if (goalCreated) {
				mState = State_Active;
			}
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

void Squad::addUnit(UnitAgent* pUnit) {
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
		// Remove unit from existing squad if it belongs to another squad.
		if (pUnit->getSquadId() != SquadId::INVALID_KEY) {
			SquadPtr pOldSquad = mpsSquadManager->getSquad(pUnit->getSquadId());
			assert(pOldSquad != NULL);

			pOldSquad->removeUnit(pUnit);
		}


		// Add unit
		mUnits.push_back(pUnit);
		pUnit->setSquadId(mId);
	
		updateUnitMovement(pUnit);

		// Set has ground or has air
		if (pUnit->isAir()) {
			mHasAirUnits = true;
		} else {
			mHasGroundUnits = true;
		}

		// Send event only if the class has been initialized (otherwise derived classes doesn't exist)
		if (mInitialized) {
			onUnitAdded(pUnit);
		}
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

		// Update has air and has ground
		if (pUnit->isAir()) {
			mHasAirUnits = false;	// testing

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

		pUnit->setSquadId(bats::SquadId::INVALID_KEY);
		onUnitRemoved(pUnit);
		pUnit->resetToDefaultBehavior();

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

const BWAPI::TilePosition& Squad::getGoalPosition() const {
	return mGoalPosition;
}

shared_ptr<Squad> Squad::getThis() const {
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
			setRegroupPosition(getCenter());
		}
	}
	// Stop regrouping if we're done
	else {
		if (finishedRegrouping()) {
			clearRegroupPosition();
		} else if (mpsGameTime->getElapsedTime() >= mRegroupStartTime + config::squad::REGROUP_NEW_POSITION_TIME &&
			isAUnitStill())
		{
			TilePosition newRegroupPosition = getCenter();

			// Only add if the new position is different
			if (newRegroupPosition != mRegroupPosition) {
				setRegroupPosition(newRegroupPosition);
			}
		}
	}
}

bool Squad::needsRegrouping() const {
	// We never need regrouping if the squad has set it to never regroup
	if (!mCanRegroup) {
		return false;
	}

	const TilePosition& center = getCenter();

	// Use same calculation both for air and ground
	for (size_t i = 0; i < mUnits.size(); ++i) {
		if (!isWithinRange(center, mUnits[i]->getUnit()->getTilePosition(), config::squad::REGROUP_DISTANCE_BEGIN)) {
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

	const TilePosition& center = getCenter();

	// Use same calculation both for air and ground
	for (size_t i = 0; i < mUnits.size(); ++i) {
		if (!isWithinRange(center, mUnits[i]->getUnit()->getTilePosition(), config::squad::REGROUP_DISTANCE_END)) {
			return false;
		}
	}

	return true;
}

void Squad::setRegroupPosition(const BWAPI::TilePosition& regroupPosition) {
	if (mRegroupPosition != regroupPosition) {
		mRegroupPosition = regroupPosition;
		mRegroupStartTime = mpsGameTime->getElapsedTime();
		updateUnitMovement();
	}
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
		Unit* pUnit = mUnits[i]->getUnit();

		if (!pUnit->isMoving() && !pUnit->isAttacking()) {
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
	double sightDistanceSquared = getSightDistance();
	sightDistanceSquared *= sightDistanceSquared;

	const TilePosition& center = getCenter();

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
				double diffDistance = static_cast<double>(getSquaredDistance(center, (*unitIt)->getTilePosition()));
				if (diffDistance <= sightDistanceSquared) {
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
	TilePosition center = getCenter();

	std::vector<Unit*> foundUnits;
	const std::set<Unit*>& units = Broodwar->getAllUnits();	
	std::set<Unit*>::const_iterator unitIt;
	for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
		// Only process enemy units
		if (Broodwar->self()->isEnemy((*unitIt)->getPlayer())) {

			// If we only want attacking units
			if (!onlyAttackingUnits || (*unitIt)->getType().canAttack()) {

				// Check if the unit is within range
				int diffDistance = getSquaredDistance(center, (*unitIt)->getTilePosition());
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
		int unitSightCurrent = mUnits[i]->getUnitType().sightRange();
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
		const string& info = getDebugInfo();

		const Position& squadCenterOnMap = Position(getCenter());

		BWAPI::Broodwar->drawTextMap(squadCenterOnMap.x(), squadCenterOnMap.y(), "%s", info.c_str());
	}
}

string Squad::getDebugInfo() const {
	stringstream ss;
	ss << TextColors::ROYAL_BLUE << left <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Id: " << mId << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Type: " << getName() << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Units: " << getUnitCount() << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Supplies: " << getSupplyCount() << "\n";

	return ss.str();
}

int Squad::getUnitCount() const {
	return static_cast<int>(mUnits.size());
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