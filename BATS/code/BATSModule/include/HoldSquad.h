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
	 * @param defendPosition the position to hold and defend, this cannot be changed later. Disband
	 * the squad and then create a new one.
	 * @param roamPosition the position the squad will roam until enemies enter the defend position
	 */
	HoldSquad(
		const std::vector<UnitAgent*>& units,
		const UnitComposition& unitComposition,
		const BWAPI::TilePosition& defendPosition,
		const BWAPI::TilePosition& roamPosition
	);

	/**
	 * Destructor
	 */
	virtual ~HoldSquad();

	/**
	 * Returns the defend position of the squad
	 * @return defend position of the squad
	 */
	 const BWAPI::TilePosition& getDefendPosition() const;

	/**
	 * Returns the roaming position of the squad
	 * @return roam position of the squad
	 */
	const BWAPI::TilePosition& getRoamPosition() const;

	virtual std::string getName() const;
	virtual void printGraphicDebugInfo() const;

protected:
	virtual void updateDerived();
	virtual std::string getDebugString() const;

private:
	BWAPI::TilePosition mDefendPosition;
};
}