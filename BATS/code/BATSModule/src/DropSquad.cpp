#include "DropSquad.h"
#include "GameTime.h"
#include "Config.h"
#include "Commander.h"
#include "Helper.h"
#include "BTHAIModule/Source/TransportAgent.h"
#include <vector>
#include <algorithm>

using namespace bats;
using namespace std;
using namespace BWAPI;

const std::string DROP_SQUAD_NAME = "DropSquad";

bool requiresLessSpace(UnitAgent* pLeftUnit, UnitAgent* pRightUnit) {
	return pLeftUnit->getUnitType().spaceRequired() < pRightUnit->getUnitType().spaceRequired();
}


DropSquad::DropSquad(
	const std::vector<UnitAgent*>& units, const UnitComposition& unitComposition) :
	AttackSquad(units, true, unitComposition)
{
	mStartTime = 0.0;
	mInitialized = false;
	mState = State_Load;
	setTravelsByAir(true);
}

DropSquad::~DropSquad() {
	// Does nothing
}

void DropSquad::computeSquadSpecificActions() {
	AttackSquad::computeSquadSpecificActions();

	if (!mInitialized) {
		mStartTime = GameTime::getInstance()->getElapsedTime();
		mInitialized = true;
		setState(State_Load);

		if (!isFull()) {
			ERROR_MESSAGE(false, "DropSquad not full when initialized, drops shall always be full at start.");
		}
	}

	switch (mState) {
	case State_Load:
		if (isTransportsDoneLoading()) {
			setState(State_Transport);
		}

		/// @todo check for loading timeout
	case State_Transport:
		// If we cannot load all attack, but only if we're not retreating
		if (travelsByGround() && !isRetreating()) {
			setState(State_Attack);
			break;
		}

		if (isEnemyAttackUnitsWithinSight()) {
				if (isEnemyFasterThanTransport()) {
					/// @todo check if the only ground is faster and where close to an edge
					/// then we can retreat
					setState(State_Attack);
				}
		}
		// No attacking enemies within sight
		else {
			// Attack when done waiting and all transports in region.
			if (!hasWaitGoals() && isTransportsInGoalRegion()) {
				setState(State_Attack);
			}

			/// @todo check if new units have been added that aren't transported, load them
			/// if that's the case
		}
		break;

	case State_Attack:
		// If we cannot load all units, i.e. must travel by ground with some. Don't try to load
		/// @todo change so that we can retreat with at least the living transport if we can.
		if (travelsByGround()) {
			break;
		}

		// Retreat when enemy units arrive, unless they are faster than us
		if (isEnemyAttackUnitsWithinSight()) {
			if (!isEnemyFasterThanTransport()) {
				setState(State_Load);
			}
			/// @todo check if the only ground is faster and where close to an edge
			/// then we can retreat
		}
		// No enemies within sight, load if we aren't within the goal region
		else {
			if (!isTransportsInGoalRegion()) {
				setState(State_Load);
			}
		}

		break;

	default:
		ERROR_MESSAGE(false, "DropSquad: Unknown state!");
		break;
	}
}

Squad::GoalStates DropSquad::checkGoalState() const {
	Squad::GoalStates goalState = Squad::GoalState_NotCompleted;

	// Only check if the goal has been initialized
	if (mInitialized) {
		// Enemy structures dead
		if (isEnemyStructuresNearGoalDead()) {
			goalState = Squad::GoalState_Succeeded;
		}
		// Timed out before reached goal
		else if (!isTransportsInGoalRegion() && hasAttackTimedOut()) {
			goalState = Squad::GoalState_Failed;
		}
	}

	/// @todo check whether we cannot land because of defended area

	return goalState;
}

void DropSquad::loadUnits() {
	vector<UnitAgent*>& units = getUnits();
	vector<UnitAgent*> unitsToLoad;

	// Get units that can be loaded
	for (size_t i = 0; i < units.size(); ++i) {
		if (TransportAgent::isValidLoadUnit(units[i])) {
			unitsToLoad.push_back(units[i]);
		}
	}

	// Sort units to load ones that requires maximum capacity first.
	std::sort(unitsToLoad.rbegin(), unitsToLoad.rend(), requiresLessSpace);


	// Clear the load queue of the transportations
	set<TransportAgent*>::iterator transportIt;
	for (transportIt = mTransports.begin(); transportIt != mTransports.end(); ++transportIt) {
		(*transportIt)->clearLoadQueue();
	}


	// Load units
	bool canLoadAll = true;
	for (size_t i = 0; i < unitsToLoad.size(); ++i) {
		bool loadedUnit = false;

		set<TransportAgent*>::iterator transportIt = mTransports.begin();
		while (!loadedUnit && transportIt != mTransports.end()) {
			// Could load the unit, or rather queue the unit
			if ((*transportIt)->loadUnit(unitsToLoad[i])) {
				loadedUnit = true;
			}
			++transportIt;
		}

		if (!loadedUnit) {
			canLoadAll = false;
		}
	}


	/// @todo make units move to transport?


	// Didn't add all units, print message
	DEBUG_MESSAGE_CONDITION(!canLoadAll, utilities::LogLevel_Info, "DropSquad::loadUnits() | " <<
		"Not enough transportations to load all units!"
	);
}

void DropSquad::unloadUnits() {
	set<TransportAgent*>::iterator transportIt;
	for (transportIt = mTransports.begin(); transportIt != mTransports.end(); ++transportIt) {
		(*transportIt)->unloadAll();
	}
}

void DropSquad::setState(States newState) {
	mState = newState;
	switch (newState) {
	case State_Attack:
		setCanRegroup(true);
		unloadUnits();
		break;

	case State_Load:
		setCanRegroup(false);
		loadUnits();
		break;

	case State_Transport:
		/// @todo update via path
		setCanRegroup(true);
		break;
	}
}

std::string DropSquad::getName() const {
	return DROP_SQUAD_NAME;
}

bool DropSquad::isEnemyFasterThanTransport() const {
	return isEnemyFasterThanTransport(getEnemyUnitsWithinSight(true));
}

bool DropSquad::isEnemyFasterThanTransport(const vector<Unit*> enemyUnits) const {
	// Get the transportations top speed
	Player* self = BWAPI::Broodwar->self();
	double transportSpeed = 0.0;
	if (self->getRace() == Races::Terran) {
		transportSpeed = self->topSpeed(UnitTypes::Terran_Dropship);
	} else if (self->getRace() == Races::Zerg) {
		transportSpeed = self->topSpeed(UnitTypes::Zerg_Overlord);
	} else if (self->getRace() == Races::Protoss) {
		transportSpeed = self->topSpeed(UnitTypes::Protoss_Shuttle);
	}

	// Check if enemy has higher speed than us
	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		Player* enemy = enemyUnits[i]->getPlayer();
		if (enemy->topSpeed(enemyUnits[i]->getType()) > transportSpeed) {
			return true;
		}
	}

	return false;
}

void DropSquad::onUnitAdded(UnitAgent* pAddedUnit) {
	if (pAddedUnit->isTransport()) {
		TransportAgent* pTransportAgent = dynamic_cast<TransportAgent*>(pAddedUnit);
		if (NULL != pTransportAgent) {
			mTransports.insert(pTransportAgent);
		} else {
			ERROR_MESSAGE(false, "Added a transport in drop that wasn't a TransportAgent!");
		}
	}
}

void DropSquad::onUnitRemoved(UnitAgent* pRemovedUnit) {
	if (pRemovedUnit->isTransport()) {
		mTransports.erase(reinterpret_cast<TransportAgent*>(pRemovedUnit));
	}
}

bool DropSquad::isTransportsInGoalRegion() const {
	set<TransportAgent*>::const_iterator transportIt;
	for (transportIt = mTransports.begin(); transportIt != mTransports.end(); ++transportIt) {
		// Transport region
		const TilePosition& transportPos = (*transportIt)->getUnit()->getTilePosition();
		BWTA::Region* transportRegion = BWTA::getRegion(transportPos);

		// Goal region
		BWTA::Region* goalRegion = BWTA::getRegion(getGoal());

		if (goalRegion != transportRegion) {
			return false;
		}
	}

	return true;
}

bool DropSquad::isTransportsDoneLoading() const {
	set<TransportAgent*>::const_iterator transportIt;
	for (transportIt = mTransports.begin(); transportIt != mTransports.end(); ++transportIt) {
		if ((*transportIt)->isLoading()) {
			return false;
		}
	}

	return true;
}

bool DropSquad::createGoal() {
	bool goalCreated = AttackSquad::createGoal();

	if (!goalCreated) {
		return false;
	}

	createViaPath();

	return true;
}

void DropSquad::createViaPath() {
	// Create a via path
	// Find closest map border to our DROP
	TilePosition dropBorderPosition = getClosestBorder(getCenter());

	// Find closest map border to the GOAL
	TilePosition goalBorderPosition;
	if (isRetreating()) {
		goalBorderPosition = getClosestBorder(getRetreatPosition());
	} else {
		goalBorderPosition = getClosestBorder(getGoal());
	}

	// Which sides do the borders lie on the map?
	Borders dropBorderSide = getAtWhichBorder(dropBorderPosition);
	Borders goalBorderSide = getAtWhichBorder(goalBorderPosition);

	// Both should be valid, else something is wrong
	if (dropBorderSide == Border_Lim || goalBorderSide == Border_Lim) {
		ERROR_MESSAGE(false, "Either drop border or goal border is not a border; they shall " <<
			"always be borders since they are calculated borders!");
	}

	list<TilePosition> viaPath;

	// If they are on the same border, use border drop, then border goal as path.
	if (dropBorderSide == goalBorderSide) {
		viaPath.push_back(dropBorderPosition);
		viaPath.push_back(goalBorderPosition);
	}
	// If they are neighbor borders use border drop, corner, then border goal as path
	if (areBordersNeighbors(dropBorderSide, goalBorderSide)) {
		viaPath.push_back(dropBorderPosition);

		// Find the corner
		TilePosition corner = getCorner(dropBorderSide, goalBorderSide);
		if (corner != TilePositions::Invalid) {
			viaPath.push_back(corner);
		} else {
			ERROR_MESSAGE(false, "Could not find a corner for the two sides, they shall always be " <<
				"corners since this was checked before!");
		}

		viaPath.push_back(goalBorderPosition);
	}
	// Else, they must be at opposite borders
	else {
		// Create two paths
		// Clockwise path
		vector<TilePosition> pathClockwise;
		pathClockwise.push_back(dropBorderPosition);
		int clockwiseBorderInt = dropBorderSide + 1;
		Borders clockwiseBorder;
		if (clockwiseBorderInt == Border_Lim) {
			clockwiseBorder = Border_First;
		} else {
			clockwiseBorder = static_cast<Borders>(clockwiseBorderInt);
		}

		// Get the two corners
		TilePosition tempCorner = getCorner(dropBorderSide, clockwiseBorder);
		if (tempCorner != TilePositions::Invalid) {
			pathClockwise.push_back(tempCorner);
		} else {
			ERROR_MESSAGE(false, "Could not find a corner between drop and clockwise border!");
		}

		tempCorner = getCorner(clockwiseBorder, goalBorderSide);
		if (tempCorner != TilePositions::Invalid) {
			pathClockwise.push_back(tempCorner);
		} else {
			ERROR_MESSAGE(false, "Could not find a corner between clockwise and goal border!");
		}

		pathClockwise.push_back(goalBorderPosition);

		// Calculate path length
		int distanceClockwise = 0;
		for (size_t i = 0; i < pathClockwise.size() - 1; ++i) {
			distanceClockwise += getSquaredDistance(pathClockwise[i], pathClockwise[i+1]);
		}


		// Counter-clockwise path
		vector<TilePosition> pathCounterClock;
		pathCounterClock.push_back(dropBorderPosition);
		int counterClockBorderInt = goalBorderSide + 1;
		Borders counterClockBorder;
		if (counterClockBorderInt == Border_Lim) {
			counterClockBorder = Border_First;
		} else {
			counterClockBorder = static_cast<Borders>(counterClockBorderInt);
		}

		// Get the two corners
		tempCorner = getCorner(dropBorderSide, counterClockBorder);
		if (tempCorner != TilePositions::Invalid) {
			pathCounterClock.push_back(tempCorner);
		} else {
			ERROR_MESSAGE(false, "Could not find a corner between drop and counter clockwise border!");
		}

		tempCorner = getCorner(counterClockBorder, goalBorderSide);
		if (tempCorner != TilePositions::Invalid) {
			pathCounterClock.push_back(tempCorner);
		} else {
			ERROR_MESSAGE(false, "Could not find a corner between counter clockwise and goal border!");
		}

		pathCounterClock.push_back(goalBorderPosition);

		// Calculate path length
		int distanceCounterClock = 0;
		for (size_t i = 0; i < pathCounterClock.size() - 1; ++i) {
			distanceCounterClock += getSquaredDistance(pathCounterClock[i], pathCounterClock[i+1]);
		}


		// Choose the shortest path
		// Clockwise
		if (distanceClockwise < distanceCounterClock) {
			viaPath.insert(viaPath.begin(), pathClockwise.begin(), pathClockwise.end());
		}
		// Counter clockwise
		else {
			viaPath.insert(viaPath.begin(), pathCounterClock.begin(), pathCounterClock.end());
		}
	}

	setViaPath(viaPath);
}

void DropSquad::onGoalFailed() {
	// Have we timed out, then retreat, else try another goal
	if (hasAttackTimedOut()) {

		// Retreat then disband
		TilePosition retreatPos = Commander::getInstance()->getRetreatPosition(getThis());

		if (retreatPos != TilePositions::Invalid) {
			setRetreatPosition(retreatPos);
			createViaPath();
			setState(State_Load);
		} else {
			/// @todo stay and fight
		}
	
	} 
	// No timeout, try another goal
	else {
		createGoal();
	}
}

void DropSquad::onGoalSucceeded() {
	// Same functionality as on Goal Failed for now
	onGoalFailed();
}

bool DropSquad::hasAttackTimedOut() const {
	return mpsGameTime->getElapsedTime() >= mStartTime + config::squad::drop::ATTACK_TIMEOUT;
}

void DropSquad::onRetreatCompleted() {
	// Unload then disband the squad
	unloadUnits();
	forceDisband();
}