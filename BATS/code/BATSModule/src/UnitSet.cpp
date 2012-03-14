#include "UnitSet.h"
#include <cassert>

using namespace bats;

UnitSet::UnitSet(const BWAPI::UnitType& unitType, int cMax, bool treatMorphAsSame) {
	assert(cMax >= 1);
	mUnitType = unitType;
	mcUnitsMax = cMax;
	mcUnitsCurrent = 0;
	mTreatMorphAsSame = treatMorphAsSame;
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

void UnitSet::clear() {
	mcUnitsCurrent = 0;
}

bool UnitSet::operator==(const BWAPI::UnitType& unitType) const {
	if (!mTreatMorphAsSame) {
		return mUnitType == unitType;
	} else {
		if ((mUnitType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode ||
			mUnitType == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
			&&
			(unitType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode ||
			unitType == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode))
		{
			return true;
		} else {
			return false;
		}
		///@todo implement other morph checks than just siege tanks
	}
}

bool operator==(const BWAPI::UnitType& unitType, const bats::UnitSet& unitSet) {
	return unitSet == unitType;
}