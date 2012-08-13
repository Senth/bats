#ifndef __MEDICAGENT_H__
#define __MEDICAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

// Forward declarations
namespace bats {
	class UnitManager;
}

/** The MedicAgent handles Terran Medics.
 *
 * @section Special Abilities
 * If in a squad it will only search for squad units and close allied units
 * If not in a squad it will search all our close units and close allied units
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 * Changed the checkUnitsToHeal() functionality
 */
class MedicAgent : public UnitAgent {
public:
	MedicAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	void computeActions();

	/**
	 * Does various checks, if the unit goes through all these checks (displayed below) the
	 * function will return true.
	 * \li organic unit
	 * \li not a worker
	 * \li not loaded into a bunker or transport
	 * \li damaged
	 * \li alive
	 * \li completed
	 * \li exists
	 * \li not itself
	 * @param unit the unit to check
	 * @return true if all checks are valid
	 */ 
	bool isMedicTarget(const BWAPI::Unit* unit) const;

private:
	/**
	 * Searches for units close to the medic to heal.
	 */
	void checkUnitsToHeal();

	static bats::UnitManager* msUnitManager;
};

#endif
