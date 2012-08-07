#include "HoldSquad.h"
#include "Config.h"
#include "Helper.h"
#include "BTHAIModule/Source/SiegeTankAgent.h"
#include <sstream>
#include <iomanip>

using namespace bats;
using namespace BWAPI;

const std::string HOLD_SQUAD_NAME = "HoldSquad";

HoldSquad::HoldSquad(
	const std::vector<UnitAgent*>& units,
	const UnitComposition& unitComposition,
	const BWAPI::TilePosition& defendPosition,
	const BWAPI::TilePosition& roamPosition)
	:
	Squad(units, false, true, unitComposition),
	mDefendPosition(defendPosition)
{
	setGoalPosition(roamPosition);
	setCanRegroup(false);
}

HoldSquad::~HoldSquad() {
	// Does nothing
}

void HoldSquad::updateDerived() {
	// Siege up siege tanks if idle and inside perimeter
	const std::vector<UnitAgent*>& units = getUnits();
	for (size_t i = 0; i < units.size(); ++i) {
		if (units[i]->isOfType(UnitTypes::Terran_Siege_Tank_Tank_Mode) &&
			units[i]->getUnit()->isIdle() &&
			isWithinRange(getGoalPosition(), units[i]->getUnit()->getTilePosition(), config::squad::defend::ROAM_PERIMETER))
		{
			SiegeTankAgent* pSiegeTank = dynamic_cast<SiegeTankAgent*>(units[i]);
			if (NULL != pSiegeTank) {
				pSiegeTank->forceSiegeMode();
			}
		}
	}

	// Find enemies within perimeter -> Set attack position
	const TilePosition& enemyPos =
		findEnemyPositionWithinRadius(mDefendPosition, config::squad::defend::DEFEND_PERIMETER);
	setTemporaryGoalPosition(enemyPos);

	/// @todo Check if a unit is outside perimeter -> make it retreat.
	/// How when we have two perimeters, roam and defend?
}

std::string HoldSquad::getName() const {
	return HOLD_SQUAD_NAME;
}

std::string HoldSquad::getDebugInfo() const {
	std::stringstream ss;
	ss << std::setw(config::debug::GRAPHICS_COLUMN_WIDTH) << "Comp.: " << getUnitComposition().getName() << "\n";

	return Squad::getDebugInfo() + ss.str();
}

const TilePosition& HoldSquad::getDefendPosition() const {
	return mDefendPosition;
}

const TilePosition& HoldSquad::getRoamPosition() const {
	return getGoalPosition();
}

void HoldSquad::printGraphicDebugInfo() const {
	Squad::printGraphicDebugInfo();

	if (config::debug::GRAPHICS_VERBOSITY == config::debug::GraphicsVerbosity_Off ||
		config::debug::modules::HOLD_SQUAD == false)
	{
		return;
	}


	// Medium
	if (config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Medium) {
		
		// Offensive perimeter
		Position defendPos(mDefendPosition);
		Broodwar->drawCircleMap(
			defendPos.x(),
			defendPos.y(),
			config::squad::defend::ENEMY_OFFENSIVE_PERIMETER * TILE_SIZE,
			BWAPI::Colors::Red
			);

		// Defend perimeter
		Broodwar->drawCircleMap(
			defendPos.x(),
			defendPos.y(),
			config::squad::defend::DEFEND_PERIMETER * TILE_SIZE,
			BWAPI::Colors::Brown
		);

		// Roam perimeter
		Position roamPos(getGoalPosition());
		Broodwar->drawCircleMap(
			roamPos.x(),
			roamPos.y(),
			config::squad::defend::ROAM_PERIMETER * TILE_SIZE,
			BWAPI::Colors::Purple
		);
	}
}