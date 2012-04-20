#include "DropSquad.h"
#include "GameTime.h"
#include "Config.h"

using namespace bats;
using namespace std;
using namespace BWAPI;

const std::string DROP_SQUAD_NAME = "DropSquad";

DropSquad::DropSquad(const std::vector<UnitAgent*>& units, const UnitComposition& unitComposition) :
	AttackSquad(units, true, unitComposition)
{
	mStartTime = 0.0;
	mInitialized = false;
	mState = State_Load;
	setCanRegroup(false);
}

DropSquad::~DropSquad() {
	// Does nothing
}

void DropSquad::computeSquadSpecificActions() {
	AttackSquad::computeSquadSpecificActions();

	if (!mInitialized) {
		mStartTime = GameTime::getInstance()->getElapsedTime();
		mInitialized = true;

		if (!isFull()) {
			ERROR_MESSAGE(false, "DropSquad not full when initialized, drops shall always be full at start.");
		}
	}

	switch (mState) {
	case State_Load:
		/// @todo check when all units have loaded
	case State_Transport:
		if (isEnemyAttackUnitsWithinSight()) {

			// If we cannot load all units just attack
			bool cannotLoadAll = travelsByGround();
			if (cannotLoadAll) {
				mState = State_Attack;
			} else {
				// Enemy is faster than us
				const std::vector<BWAPI::Unit*>& enemyUnits = getEnemyUnitsWithinSight(true);
				if (!enemyIsFasterThanTransport(enemyUnits)) {
					/// @todo check if the only ground is faster and where close to an edge
					/// then we can retreat
					mState = State_Attack;
				}

			}
		}
		// No attacking enemies within sight
		else {

		}
		break;

	case State_Attack:
		// Always try to unload the units
		unloadUnits();
		break;

	default:
		ERROR_MESSAGE(false, "DropSquad: Unknown state!");
	}
}

Squad::GoalStates DropSquad::checkGoalState() const {
	Squad::GoalStates goalState = Squad::GoalState_NotCompleted;

	// Enemy structures dead
	if (isEnemyStructuresNearGoalDead()) {
		goalState = Squad::GoalState_Succeeded;
	} else if (mStartTime + config::squad::drop::TIMEOUT >= GameTime::getInstance()->getElapsedTime()) {
		goalState = Squad::GoalState_Failed;
	}

	/// @todo check whether we cannot land because of defended area

	return goalState;
}

void DropSquad::loadUnits() {
	vector<UnitAgent*>& units = getUnits();

	// Includes the number of free spots for the transport
	vector<pair<UnitAgent*, int>> transports;
	vector<UnitAgent*> groundUnits;

	// Split units into groups for easier handling
	for (size_t i = 0; i < units.size(); ++i) {
		if (units[i]->isTransport()) {
			transports.push_back(make_pair(units[i], units[i]->getUnitType().spaceProvided()));
		} else if (units[i]->isGround()) {
			groundUnits.push_back(units[i]);
		}
	}

	bool allAdded = true;

	// Find transportations for all the units
	for (size_t groundUnitIndex = 0; groundUnitIndex < groundUnits.size(); ++groundUnitIndex) {
		BWAPI::Unit* pGroundUnit = groundUnits[groundUnitIndex]->getUnit();

		bool added = false;
		vector<pair<UnitAgent*, int>>::iterator transportIt = transports.begin();

		// Find a transport that can carry the unit
		while (!added && transportIt != transports.end()) {
			// Add to transport if it has free space for this unit
			if (transportIt->second >= pGroundUnit->getType().spaceRequired()) {
				transportIt->first->getUnit()->load(pGroundUnit,true);
				added = true;
			} else {
				++transportIt;
			}
		}

		if (!added) {
			allAdded = false;
		}
		
	}

	// Didn't add all units, print message
	DEBUG_MESSAGE_CONDITION(!allAdded, utilities::LogLevel_Info, "DropSquad::loadUnits() | " <<
		"Not enough transportations to load all units!"
	);
}

void DropSquad::unloadUnits() {
	// Rather than spamming commands, only try to unload transports that are actually doing something
	vector<UnitAgent*>& units = getUnits();
	set<Unit*> transportsToUnload;

	for (size_t i = 0; i < units.size(); ++i) {
		if (units[i]->getUnit()->isLoaded()) {
			transportsToUnload.insert(units[i]->getUnit()->getTransport());
		}
	}
	
	set<Unit*>::iterator transportIt;
	for (transportIt = transportsToUnload.begin(); transportIt != transportsToUnload.end(); ++transportIt) {
		(*transportIt)->unloadAll();
	}
}

std::string DropSquad::getName() const {
	return DROP_SQUAD_NAME;
}

bool DropSquad::enemyIsFasterThanTransport(const vector<Unit*> enemyUnits) const {
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