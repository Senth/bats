#include "HoldSquad.h"
#include "Config.h"
#include "Helper.h"
#include "BTHAIModule/Source/SiegeTankAgent.h"
#include <sstream>
#include <iomanip>

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
	setCanRegroup(false);
}

HoldSquad::~HoldSquad() {
	// Does nothing
}

void HoldSquad::updateDerived() {
	// Siege up siege tanks if idle and inside perimeter
	const std::vector<UnitAgent*>& units = getUnits();
	for (size_t i = 0; i < units.size(); ++i) {
		if (units[i]->isOfType(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode) &&
			units[i]->getUnit()->isIdle() &&
			isWithinRange(getGoalPosition(), units[i]->getUnit()->getTilePosition(), config::squad::defend::PERIMETER))
		{
			SiegeTankAgent* pSiegeTank = dynamic_cast<SiegeTankAgent*>(units[i]);
			if (NULL != pSiegeTank) {
				pSiegeTank->forceSiegeMode();
			}
		}
	}

	// Find enemies within perimeter -> Set attack position
	setTemporaryGoalPosition(findEnemyPositionWithinRadius(getGoalPosition(), config::squad::defend::PERIMETER));

	/// @todo Check if a unit is outside perimeter -> make it retreat.
}

std::string HoldSquad::getName() const {
	return HOLD_SQUAD_NAME;
}

std::string HoldSquad::getDebugInfo() const {
	std::stringstream ss;
	ss << std::setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Comp.: " << getUnitComposition().getName() << "\n";

	return Squad::getDebugInfo() + ss.str();
}