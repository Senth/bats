#include "UnitComposition.h"
#include "Utilities/Logger.h"

using namespace bats;

const int PRIORITY_MIN = 0;
const int PRIORITY_MAX = 100;

UnitComposition::UnitComposition() {
	clear();
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
	// Needs a type, name, and at least one unit set.
	return !mUnitSets.empty() && mName != "" && mType != "" ;
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
	// found then return directly with the remove result
	while (i < mUnitSets.size()) {
		if (mUnitSets[i] == pUnit->getUnitType()) {
			bool removeOk = mUnitSets[i].removeUnits(1);
			return removeOk;
		}

		++i;
	}

	return false;
}

int UnitComposition::getPriority() const {
	return mPriority;
}

const std::string& UnitComposition::getName() const {
	return mName;
}

const std::string& UnitComposition::getType() const {
	return mType;
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

bool UnitComposition::operator<(const UnitComposition& rightComposition) const {
	return mPriority < rightComposition.mPriority;
}

void UnitComposition::addUnitSet(const UnitSet& unitSet) {
	mUnitSets.push_back(unitSet);
}

void UnitComposition::resetUnitCountToZero() {
	for (size_t i = 0; i < mUnitSets.size(); ++i) {
		mUnitSets[i].resetUnitCountAsZero();
	}
}

void UnitComposition::clear() {
	mPriority = PRIORITY_MIN;
	mName.clear();
	mType.clear();
	mUnitSets.clear();
}

void UnitComposition::setTypeAndName(const std::string& type, const std::string& name) {
	mType = type;
	mName = name;
}

void UnitComposition::setPriority(int priority) {
	ERROR_MESSAGE_CONDITION(priority < PRIORITY_MIN, false, 
		"Setting too low priority for UnitComposition. Type: " << mType << ", name: " << mName <<
		", priority: " << priority << ". Minimimum priority allowed is " << PRIORITY_MIN);

	ERROR_MESSAGE_CONDITION(priority > PRIORITY_MAX, false, 
		"Setting too high priority for UnitComposition. Type: " << mType << ", name: " << mName <<
		", priority: " << priority << ". Maximum priority allowed is " << PRIORITY_MAX);

	mPriority = priority;
}