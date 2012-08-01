#include "HoldSquad.h"
#include "Config.h"
#include "Helper.h"
#include "BTHAIModule/Source/SiegeTankAgent.h"

using namespace bats;

const std::string HOLD_SQUAD_NAME = "HoldSquad";

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
	// Siege up siege tanks if idle and inside perimeter
	const std::vector<UnitAgent*>& units = getUnits();
	for (size_t i = 0; i < units.size(); ++i) {
		if (units[i]->isOfType(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode) &&
			units[i]->getUnit()->isIdle())
		{
			SiegeTankAgent* pSiegeTank = dynamic_cast<SiegeTankAgent*>(units[i]);
			if (NULL != pSiegeTank) {
				pSiegeTank->forceSiegeMode();
			}
		}
	}

	// Find enemies within perimeter -> Set attack position
	setTemporaryGoalPosition(findEnemyPositionWithinRadius(getGoalPosition(), config::squad::defend::PERIMETER));

	/// @todo Check if a units is outside perimeter, make it retreat.
}

std::string HoldSquad::getName() const {
	return HOLD_SQUAD_NAME;
}