#pragma once

#include "Squad.h"

// Namespace for the project
namespace bats {

/**
 * A squad that holds a specified position. If any enemies enter the defensive area the
 * squad will attack it. The defensive area is specified by config::squad::defense::PERIMETER.
 * The squad will set tanks to siege mode and hold the siege. Units are not allowed to leave
 * the defended area, not even when attacking or retreating.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class HoldSquad : public Squad {
public:
	/**
	 * Constructs the squad with specified unit composition.
	 * @param units list of possible units to add
	 * @param unitComposition the unit composition the squad uses
	 * @param holdPosition the position to hold, this cannot be changed later. Disband
	 * the squad and then create a new one.
	 */
	HoldSquad(
		const std::vector<UnitAgent*>& units,
		const UnitComposition& unitComposition,
		const BWAPI::TilePosition& holdPosition
	);

	/**
	 * Destructor
	 */
	virtual ~HoldSquad();

protected:
	virtual void updateDerived();
	virtual void onUnitAdded(UnitAgent* pAddedUnit);
	virtual void onUnitRemoved(UnitAgent* pRemovedUnit);

private:

};
}