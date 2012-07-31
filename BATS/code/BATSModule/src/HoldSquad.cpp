#include "HoldSquad.h"

using namespace bats;

HoldSquad::HoldSquad(
	const std::vector<UnitAgent*>& units,
	const UnitComposition& unitComposition,
	const BWAPI::TilePosition& holdPosition)
	:
	Squad(units, false, true, unitComposition)
{
	setGoalPosition(holdPosition);
}

HoldSquad::~HoldSquad() {
	// Does nothing
}

void HoldSquad::updateDerived() {
	/// @todo
	// Siege up siege tanks if units are idle
	
	// Find enemies within perimeter

	// Check if a units is outside perimeter, make it retreat.
}

#pragma warning(push)
#pragma warning(disable:4100)
void HoldSquad::onUnitAdded(UnitAgent* pAddedUnit) {
	/// @todo disable auto-siege on siege tanks
}

void HoldSquad::onUnitRemoved(UnitAgent* pRemovedUnit) {
	/// @todo enable auto-siege on siege tanks
}
#pragma warning(pop)