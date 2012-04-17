#include "DropSquad.h"
#include "GameTime.h"

using namespace bats;
using namespace std;

const std::string DROP_SQUAD_NAME = "DropSquad";

DropSquad::DropSquad(const std::vector<UnitAgent*>& units, const UnitComposition& unitComposition) :
	AttackSquad(units, true, unitComposition)
{
	mStartTime = 0.0;
	mLoaded = false;
	mInitialized = false;
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


		loadUnits();
	}

	// Check whether we're close to the goal so that we can unload?
	if (!hasWaitGoals()) {

	}

	
	

	// Check if we shall load again

	// Check for timeout
}

Squad::GoalStates DropSquad::checkGoalState() const {
	return GoalState_NotCompleted;
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
				transportIt->first->getUnit()->load(pGroundUnit);
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

std::string DropSquad::getName() const {
	return DROP_SQUAD_NAME;
}