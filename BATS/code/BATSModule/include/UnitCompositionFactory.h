#pragma once

#include "UnitComposition.h"
#include <vector>

// Namespace for the project
namespace bats {

/**
 * Creates UnitCompositions that other classes can use. Reads from the UnitComposition
 * directory in BATS-data for what configurations to use.
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class UnitCompositionFactory {
public:
	/**
	 * Returns the instance of UnitCompositionFactory. If it is the first time
	 * getInstance() is called it will automatically load all settings from the
	 * configuration files.
	 * @return instance of UnitCompositionFactory
	 */
	static UnitCompositionFactory* getInstance();

	/**
	 * Destructor
	 */
	virtual ~UnitCompositionFactory();

	/**
	 * Reloads all UnitSets from the configuration file. Use this function
	 * when you have changed something in the unit set files and want to
	 * reload these settings without restarting StarCraft.
	 */
	void reloadConfigs();

	/**
	 * Returns all unit compositions that are available for the specified type.
	 * If you want to return all available unit compositions independent of the type,
	 * use UnitComposition_All instead. This function wants all available units and
	 * then returns only unit compositions that can be filled with these units.
	 * @param availableUnits all available units that can be used to fill the unit
	 * compositions. Note the units do not have to be free units, they can belong to
	 * a squad if you want to split up your squad.
	 * @param type the unit composition type, specify UnitComposition_All to use test
	 * all unitCompositions
	 * @return all unit compositions that are available for the specified type. Empty
	 * vector if no available unit compositions were found.
	 */
	std::vector<UnitComposition> getUnitCompositionsByType(
		const std::vector<UnitAgent*>& availableUnits,
		UnitCompositions type
	) const;

	/**
	 * An invalid unit composition.
	 */
	static UnitComposition INVALID;
private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	UnitCompositionFactory();

	static UnitCompositionFactory* mpsInstance;
	std::vector<std::vector<UnitComposition> > mUnitCompositions;
};
}