#include "UnitComposition.h"

using namespace bats;

UnitComposition::UnitComposition() : mValid(false) {
	// Does nothing
}

UnitComposition::UnitComposition(
	const std::string& type,
	const std::string& name,
	const std::vector<UnitSet>& unitSets) :
		mType(type),
		mName(name),
		mUnitSets(unitSets),
		mValid(true)
{
		// Does nothing
}

UnitComposition::~UnitComposition() {
	// Does nothing
}

bool UnitComposition::isFull() const {
	for (size_t i = 0; i < mUnitSets.size(); ++i) {
		if (!mUnitSets[i].isFull()) {
			return false;
		}
	}

	return true;
}

bool UnitComposition::isValid() const {
	return mValid;
}

std::vector<const UnitSet> UnitComposition::getUnitSets() const {
	std::vector<const UnitSet> vacantSets;

	for (size_t i = 0; i < mUnitSets.size(); ++i) {
		if (!mUnitSets[i].isFull()) {
			vacantSets.push_back(mUnitSets[i]);
		}
	}

	return vacantSets;
}

bool UnitComposition::addUnit(UnitAgent* pUnit) {
	size_t i = 0;

	// Find the unit type and add one unit, if it is
	// found then return directly with add result
	while (i < mUnitSets.size()) {
		if (mUnitSets[i] == pUnit->getUnitType()) {
			bool addOk = mUnitSets[i].addUnits(1);
			return addOk;
		}

		++i;
	}

	return false;
}

bool UnitComposition::removeUnit(UnitAgent* pUnit) {
	size_t i = 0;

	// Find the unit type and remove one unit, if it is
	// found then return directyl with the remove result
	while (i < mUnitSets.size()) {
		if (mUnitSets[i] == pUnit->getUnitType()) {
			bool removeOk = mUnitSets[i].removeUnits(1);
			return removeOk;
		}

		++i;
	}

	return false;
}

std::vector<UnitAgent*> UnitComposition::addUnits(const std::vector<UnitAgent*>& units) {
	// Try to add all units, those that can't be added will be placed in the notAddedUnits
	// vector and returned.
	std::vector<UnitAgent*> notAddedUnits;

	for (size_t i = 0; i < units.size(); ++i) {
		bool addOk = addUnit(units[i]);
		if (!addOk) {
			notAddedUnits.push_back(units[i]);
		}
	}

	return notAddedUnits;
}

std::vector<UnitAgent*> UnitComposition::removeUnits(const std::vector<UnitAgent*>& units) {
	// Try to remove all units, those that can't be added will be placed in the notRemovedUnits
	// vector and returned
	std::vector<UnitAgent*> notRemovedUnits;

	for (size_t i = 0; i < units.size(); ++i) {
		bool removeOk = removeUnit(units[i]);
		if (!removeOk) {
			notRemovedUnits.push_back(units[i]);
		}
	}

	return notRemovedUnits;
}

void UnitComposition::clear() {
	for (size_t i = 0; i < mUnitSets.size(); ++i) {
		mUnitSets[i].clear();
	}
}