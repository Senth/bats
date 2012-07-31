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
	UnitComposition_Defend,
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
	 * UnitSet::getSlotsVacant() to see how many units are missing for each unit set.
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
	 * Returns the priority of the UnitComposition. If no priority has been set
	 * this defaults to 0, higher value means higher priority.
	 * @return priority of the UnitComposition.
	 */
	int getPriority() const;

	/**
	 * Returns the name of the composition
	 * @return name of the composition
	 */
	const std::string& getName() const;

	/**
	 * Returns the type of the composition
	 * @return type of the composition
	 */
	const std::string& getType() const;

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

	/**
	 * Operator that checks if this unit composition has lower priority then the one
	 * on the right side.
	 * @param rightComposition the composition on the right side of the less '<' sign.
	 * @return true if this composition has less priority
	 */
	bool operator<(const UnitComposition& rightComposition) const;
	
private:
	/**
	 * Constructs an invalid unit composition. To make the UnitComposition valid it needs
	 * to have at least one UnitSet, a type, and a name. The priority will default to
	 * 0 if none are set. To reset the state of the of the UnitComposition to the state directly
	 * after this constructor have been called, i.e. invalid, call clear().
	 * @see addUnitSet() to add a unit set, at least one needs to be added.
	 * @see setTypeAndName() to set the type and name of the unit set.
	 * @see setPriority() to set the priority of the unit composition (optional).
	 */
	UnitComposition();

	/**
	 * Set the unit sets this composition has. Used by UnitCompositionFactory
	 * @param unitSet what unit type and how many of the unit the unit composition contains.
	 */
	void addUnitSet(const UnitSet& unitSet);

	/**
	 * Resets all the number of units to 0 for all unit sets. Helper function for
	 * UnitCompositionFactory
	 */
	void resetUnitCountToZero();

	/**
	 * Clears the unit sets to 0, sets an invalid name and type, resets the priority to 0.
	 * This will cause the unitComposition to be invalid.
	 */
	void clear();

	/**
	 * Sets the name and the type of the unit composition
	 * @param type what type the unit set is of. E.g. drop.
	 * @param name name of the unit set. E.g. marine.
	 */
	void setTypeAndName(const std::string& type, const std::string& name);

	/**
	 * Sets the priority of the unit composition. Higher value means higher priority.
	 * @param priority the priority of the unit composition. Needs to be in the range [0,100].
	 */
	void setPriority(int priority);

	friend class UnitCompositionFactory;

	std::string mType;
	std::string mName;
	int mPriority;
	std::vector<UnitSet> mUnitSets;
};
}