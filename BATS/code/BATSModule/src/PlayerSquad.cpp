#include "PlayerSquad.h"
#include "GameTime.h"
#include "Config.h"
#include "Helper.h"
#include <iomanip>
#include <sstream>

using namespace bats;
using namespace BWAPI;
using namespace std;

const int MAX_KEYS = 2000;

const GameTime* PlayerSquad::mpsGameTime = NULL;
utilities::KeyHandler<_PlayerSquadType>* PlayerSquad::mpsKeyHandler = NULL;
int PlayerSquad::mcsInstances = 0;

PlayerSquad::PlayerSquad() {
	if (mpsGameTime == NULL) {
		mpsGameTime = GameTime::getInstance();
	}

	if (mcsInstances == 0) {
		utilities::KeyHandler<_PlayerSquadType>::init(MAX_KEYS);
		mpsKeyHandler = utilities::KeyHandler<_PlayerSquadType>::getInstance();
	}
	mcsInstances++;

	mId = mpsKeyHandler->allocateKey();

	// Add listener
	config::addOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE), this);

	mUpdateLast = 0.0;
}

PlayerSquad::~PlayerSquad() {
	// Remove listener
	config::removeOnConstantChangedListener(TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE), this);

	mpsKeyHandler->freeKey(mId);

	// Delete KeyHandler if no squads are available
	mcsInstances--;
	if (mcsInstances == 0) {
		SAFE_DELETE(mpsKeyHandler);
	}
}

void PlayerSquad::update() {
	// Skip if no units in squad
	if (mUnits.empty()) {
		return;
	}

	// Check if it has passed the interval time
	if (mpsGameTime->getElapsedTime() - mUpdateLast < config::classification::squad::MEASURE_INTERVAL_TIME) {
		return;
	}
	mUpdateLast = mpsGameTime->getElapsedTime();

	updateSupply();
	updateCenter();
	updateDerived();
}

int PlayerSquad::getSupplyCount() const {
	int cSupply = 0;
	for (size_t i = 0; i < mUnits.size(); ++i) {
		cSupply += mUnits[i]->getType().supplyRequired();
	}

	return cSupply;
}

int PlayerSquad::getDeltaSupplyCount() const {
	/// @todo what if squads merges or splits, this will be quite inaccurate.
	if (mSupplies.size() == config::classification::squad::MEASURE_SIZE) {
		return mSupplies.front() - mSupplies.back();
	} else {
		return 0;
	}
}

size_t PlayerSquad::getUnitCount() const {
	return mUnits.size();
}

bool PlayerSquad::isEmpty() const {
	return mUnits.size() == 0;
}

void PlayerSquad::addUnit(const BWAPI::Unit* unit) {
	mUnits.push_back(unit);
}

void PlayerSquad::removeUnit(const BWAPI::Unit* unit) {
	std::vector<const BWAPI::Unit*>::iterator it = mUnits.begin();
	bool found = false;
	while (it != mUnits.end() && !found) {
		if (*it == unit) {
			found = true;
			it = mUnits.erase(it);
		} else {
			++it;
		}
	}
}

const std::vector<const BWAPI::Unit*>& PlayerSquad::getUnits() const {
	return mUnits;
}

PlayerSquadId PlayerSquad::getId() const {
	return mId;
}

int PlayerSquad::getMaxKeys() {
	return MAX_KEYS;
}

BWAPI::TilePosition PlayerSquad::getDirection() const {
	if (mCenter.size() < config::classification::squad::MEASURE_SIZE) {
		return BWAPI::TilePositions::Invalid;
	} else {
		return mCenter.front() - mCenter.back();
	}
}

int PlayerSquad::getDistanceTraveledSquared() const {
	if (mCenter.size() < config::classification::squad::MEASURE_SIZE) {
		return 0;
	} else {
		return getSquaredDistance(mCenter.front(), mCenter.back());
	}
}

void PlayerSquad::updateCenter() {
	TilePosition center(0,0);

	for (size_t i = 0; i < mUnits.size(); ++i) {
		TilePosition unitPos = mUnits[i]->getTilePosition();
		if (unitPos.isValid()) {
			center += unitPos;
		}
	}

	if (mUnits.size() > 0) {
		center.x() /= mUnits.size();
		center.y() /= mUnits.size();
	}

	mCenter.push_front(center);

	// Delete the oldest (if full)
	if (config::classification::squad::MEASURE_SIZE < mCenter.size()) {
		mCenter.pop_back();
	}
}

const TilePosition& PlayerSquad::getCenter() const {
	if (!mCenter.empty()) {
		return mCenter.front();
	} else {
		return TilePositions::Invalid;
	}
}

TilePosition PlayerSquad::getTargetPosition() const {
	// Get common target for majority of the units
	map<TilePosition, int> positions;

	for (size_t i = 0; i < mUnits.size(); ++i) {
		TilePosition targetPosition = TilePosition(mUnits[i]->getTargetPosition());
		if (targetPosition != TilePositions::Invalid) {
			positions[targetPosition]++;
		}
	}

	// Return the position which most unit use
	int cMaxUnits = 0;
	TilePosition mostTargetedPosition = TilePositions::Invalid;

	map<TilePosition, int>::const_iterator positionIt;
	for (positionIt = positions.begin(); positionIt != positions.end(); ++positionIt) {
		if (positionIt->second > cMaxUnits) {
			cMaxUnits = positionIt->second;
			mostTargetedPosition = positionIt->first;
		}
	}

	return mostTargetedPosition;
}

bool PlayerSquad::belongsToThisSquad(BWAPI::Unit* unit) const {
	for (size_t i = 0; i < mUnits.size(); ++i) {
		if (mUnits[i] == unit) {
			return true;
		}
	}

	return false;
}

void PlayerSquad::printGraphicDebugInfo() const {
	// Skip if not turned on
	if (config::debug::GRAPHICS_VERBOSITY == config::debug::GraphicsVerbosity_Off || !isDebugOn())
	{
		return;
	}

	// Low
	// Print id, state, number of units and number of supplies.
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Low) {
		if (!mCenter.empty()) {
			BWAPI::Position squadCenterOnMap = BWAPI::Position(mCenter.front());

			BWAPI::Broodwar->drawTextMap(squadCenterOnMap.x(), squadCenterOnMap.y(), "%s", getDebugString().c_str());
		}
	}


	// Medium
	// Draw line from the front and back center, display the length of this line
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
		if (!mCenter.empty()) {
			pair<Position, Position> squadMovement = make_pair(mCenter.front(), mCenter.back());

			// Length
			double length = (mCenter.front() - mCenter.back()).getLength();

			// Draw line
			Broodwar->drawLineMap(
				squadMovement.first.x(), squadMovement.first.y(),
				squadMovement.second.x(), squadMovement.second.y(),
				Colors::Purple
				);

			int xOffset = -64;

			// Draw text in back of line
			Broodwar->drawTextMap(
				squadMovement.second.x() + xOffset, squadMovement.second.y(),
				"%sLength: %g",
				TextColors::PURPLE.c_str(),
				length
				);

		}
	}
}

string PlayerSquad::getDebugString() const {
	stringstream ss;
	ss << TextColors::DARK_GREEN << left << setprecision(2) <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Id: " << getId() << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Units: " << getUnitCount() << "\n" <<
		setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Supplies: " << getSupplyCount() << "\n";

	return ss.str();
}

void PlayerSquad::onConstantChanged(config::ConstantName constanName) {
	// measure_time
	if (constanName == TO_CONSTANT_NAME(config::classification::squad::MEASURE_SIZE)) {
		// If less then erase those at the back
		if (config::classification::squad::MEASURE_SIZE < mCenter.size()) {
			mCenter.resize(config::classification::squad::MEASURE_SIZE);
		}
		if (config::classification::squad::MEASURE_SIZE < mSupplies.size()) {
			mSupplies.resize(config::classification::squad::MEASURE_SIZE);
		}
	}
}

bool PlayerSquad::isMeasureFull() const {
	return config::classification::squad::MEASURE_SIZE == mCenter.size();
}

void PlayerSquad::updateSupply() {
	mSupplies.push_front(getSupplyCount());

	if (mSupplies.size() > config::classification::squad::MEASURE_SIZE) {
		mSupplies.pop_back();
	}
}