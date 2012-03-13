#pragma once

#include <string>
#include "BTHAIModule/Source/UnitAgent.h"
#include "UnitSet.h"

// Namespace for the project
namespace bats {

class UnitCompositionFactory;

/**
 * Enumeration of all UnitComposition types
 */
enum UnitCompositions {
	UnitComposition_First = 0,
	UnitComposition_Drop = UnitComposition_First,
	UnitComposition_Scout,
	UnitComposition_Harass,
	UnitComposition_All,
	UnitComposition_Lim
};

/**
 * A composition of unit sets that creates a group of units that
 * can be used for different squads. Only UnitCompositionFactory can create
 * a new UnitComposition.
 * 
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class UnitComposition {
public:
	/**
	 * Destructor
	 */
	virtual ~UnitComposition();

	/**
	 * Checks whether the unit composition is full or not.
	 * @return true if the unit composition is full
	 */
	bool isFull() const;

	/**
	 * Checks whether this unit composition is valid, i.e. it has UnitSets.
	 * @return true if this unit composition is valid, else false.
	 */
	bool isValid() const;

	/**
	 * Returns all the unit sets with vacant slots, thus you can call
	 * UnitSets::getSlotsVacant() to see how many units are missing for each unit set.
	 * @return all unit sets with vacant slots.
	 * @note you cannot add units to these unit sets directly, but have
	 * to add it through UnitComposition's addUnits(), the same goes for removing
	 * units
	 */
	std::vector<const UnitSet> getUnitSets() const;

	/**
	 * Tries to add a unit to the unit composition.
	 * @param pUnit the unit we're trying to add.
	 * @return true if the unit was successfully added; false if all slots are occupied
	 * for this unit type.
	 */
	bool addUnit(UnitAgent* pUnit);

	/**
	 * Tries to remove a unit from the unit composition.
	 * @param pUnit the unit we're trying to remove.
	 * @return true if the unit was successfully removed; false if no units of this type exists.
	 */
	bool removeUnit(UnitAgent* pUnit);

	/**
	 * Tries to add all units to the unit composition.
	 * @param units all the units we're trying to add.
	 * @return an empty vector if all units were succussfully added; if some units failed
	 * to be added to the unit set, because all slots are occupied, these will be returned
	 * in the vector.
	 */
	std::vector<UnitAgent*> addUnits(const std::vector<UnitAgent*>& units);

	/**
	 * Tries to remove all units from the unit composition.
	 * @param units all the units we're trying to remove.
	 * @return an empty vector if all units were successfully removed; if some units failed
	 * to be removed from the unit set, because no units of these types exists, these will be
	 * returned in the vector.
	 */
	std::vector<UnitAgent*> removeUnits(const std::vector<UnitAgent*>& units);
	
private:
	/**
	 * Constructs a new UnitComposition with a type (drop), name (marine), and all
	 * the UnitSets that are needed for this UnitComposition (1 dropship, 8 marines).
	 * The constructor is private to ensure that only UnitCompositionFactory can create
	 * new UnitCompositions.
	 * @param type name of the unit composition type, e.g. (drop)
	 * @param name the name of the unit composition. A more descriptive name what the type
	 * contains, e.g. marines or marine-medics.
	 * @param unitSets what unit types and how many the unit composition contains.
	 */
	UnitComposition(
		const std::string& type,
		const std::string& name,
		const std::vector<UnitSet>& unitSets
	);

	/**
	 * Constructs an invalid unit composition.
	 */
	UnitComposition();

	friend class UnitCompositionFactory;

	std::string mType;
	std::string mName;
	std::vector<UnitSet> mUnitSets;
	bool mValid;
};
}