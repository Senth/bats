#include "UnitSet.h"
#include <cassert>

using namespace bats;

UnitSet::UnitSet(const BWAPI::UnitType& unitType, int cMax, int cCurrent) {
	assert(cMax >= cCurrent);
	mUnitType = unitType;
	mcUnitsMax = cMax;
	mcUnitsCurrent = cCurrent;
}

UnitSet::~UnitSet() {
	// Does Nothing
}

int UnitSet::getSlotsVacant() const {
	return mcUnitsMax - mcUnitsCurrent;
}

int UnitSet::getSlotsOccupied() const {
	return mcUnitsCurrent;
}

int UnitSet::getSlotsMax() const {
	return mcUnitsMax;
}

bool UnitSet::isFull() const {
	return getSlotsVacant() == 0;
}

const BWAPI::UnitType& UnitSet::getUnitType() const {
	return mUnitType;
}

bool UnitSet::addUnits(int cUnits) {
	if (cUnits <= getSlotsVacant()) {
		mcUnitsCurrent += cUnits;
		return true;
	} else {
		return false;
	}
}

bool UnitSet::removeUnits(int cUnits) {
	if (cUnits <= getSlotsOccupied()) {
		mcUnitsCurrent -= cUnits;
		return true;
	} else {
		return false;
	}
}

bool UnitSet::operator==(const BWAPI::UnitType& unitType) const {
	return mUnitType == unitType;
}

bool operator==(const BWAPI::UnitType& unitType, const bats::UnitSet& unitSet) {
	return unitSet.getUnitType() == unitType;
}