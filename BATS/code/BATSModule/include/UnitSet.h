#pragma once

#include <BWAPI/UnitType.h>

// Namespace for the project
namespace bats {

// Forward declarations
class UnitCompositionFactory;

/**
 * Handles the number of units of a specified type a squad needs.
 * For squads that can be full. Effectively keeps a count of how many units we
 * need to be full.
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class UnitSet {
public:
	/**
	 * Destructor
	 */
	virtual ~UnitSet();

	/**
	 * Returns the number of vacant slots of the instance.
	 * @return number of vacant slots of the instance.
	 */
	int getSlotsVacant() const;

	/**
	 * Returns the number of occupied slots of the instance.
	 * @return number of occupied slots of the instance.
	 */
	int getSlotsOccupied() const;

	/**
	 * Returns the maximum number of units the instance can have.
	 * @return maximum number of units the instance can have.
	 */
	int getSlotsMax() const;

	/**
	 * Checks whether the UnitSet is full or not, equivalent to
	 * getFreeSlots() == 0.
	 * @return true if the UnitSet is full
	 */
	bool isFull() const;

	/**
	 * Returns the UnitType we keep count on.
	 * @return the UnitType we keep count on.
	 */
	const BWAPI::UnitType& getUnitType() const;

	/**
	 * Adds the specified number of units to the UnitSet. If you
	 * try to add more units than the number of free spots no units
	 * are added and the function returns false.
	 * @param cUnits number of units to add.
	 */
	bool addUnits(int cUnits);

	/**
	 * Removes the specified number of units from the UnitSet. If you
	 * try to remove more units than the instance has.
	 * @param cUnits number of units to remove.
	 */
	bool removeUnits(int cUnits);

	/**
	 * Clears all current units from this UnitSet.
	 */
	void clear();

	/**
	 * Checks whether the instance contains and keeps track of units of the
	 * specified UnitType.
	 * @param unitType the UnitType to test with.
	 * @return true if this instance contains unitType.
	 */
	bool operator==(const BWAPI::UnitType& unitType) const;
	
private:
	friend class UnitCompositionFactory;

	/**
	 * Creates a UnitSet with the specified UnitType and how many of these units
	 * shall exist. This information can not be changed later.
	 * @param unitType the type of unit we want to specify a limit on
	 * @param cMax how many units we max can have
	 * @param treatMorphAsSame set this to true if the unit set should treat morphed units
	 * as same, e.g. "Terran Siege Tank Tank Mode" and "Terran Siege Tank Siege Mode" will
	 * be treated as the same unit. Defaults to false
	 * @todo only the Siege Tank check has been implemented at the moment. Implement the rest.
	 */
	UnitSet(const BWAPI::UnitType& unitType, int cMax, bool treatMorphAsSame = false);

	BWAPI::UnitType mUnitType;
	bool mTreatMorphAsSame;
	int mcUnitsMax;
	int mcUnitsCurrent;
};
}

/**
 * Checks whether the UnitSet contains and keeps track of units from the specified UnitType
 * @param unitType the UnitType to check with
 * @param unitSet the UnitSet to check if it contains unitType.
 * @return true if the UnitSet contains the specified unitType.
 */
bool operator==(const BWAPI::UnitType& unitType, const bats::UnitSet& unitSet);