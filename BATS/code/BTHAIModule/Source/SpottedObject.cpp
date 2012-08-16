#include "SpottedObject.h"

using namespace BWAPI;
using namespace std;

SpottedObject::SpottedObject() {
	mPosition = Positions::Invalid;
	mTilePosition = TilePositions::Invalid;
	mType = UnitTypes::None;
	mUnitId = -1;
	mActive = true;
}

SpottedObject::SpottedObject(const Unit* unit) {
	mType = unit->getType();
	mPosition = unit->getPosition();
	mTilePosition = unit->getTilePosition();
	mUnitId = unit->getID();
	mActive = true;
}

SpottedObject::SpottedObject(const Position& pos) {
	mPosition = pos;
	mTilePosition = TilePosition(pos);
	mType = UnitTypes::None;
	mUnitId = -1;
	mActive = true;
}

bool SpottedObject::isActive() const {
	return mActive;
}

void SpottedObject::setInactive() {
	mActive = false;
}

int SpottedObject::getUnitID() const {
	return mUnitId;
}

const UnitType& SpottedObject::getType() const {
	return mType;
}

const Position& SpottedObject::getPosition() const {
	return mPosition;
}

const TilePosition& SpottedObject::getTilePosition() const {
	return mTilePosition;
}

bool SpottedObject::isAt(const TilePosition& tilePos) const {
	return tilePos == mTilePosition;
}
