#include "MedicAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "Utilities/Logger.h"
#include "BATSModule/include/Config.h"
#include "BATSModule/include/SquadManager.h"
#include "BATSModule/include/Helper.h"
#include "BATSModule/include/Squad.h"
#include "BatsModule/include/UnitManager.h"

using namespace BWAPI;
using namespace std;

bats::UnitManager* MedicAgent::msUnitManager = NULL;

MedicAgent::MedicAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "MedicAgent";

	if (msUnitManager == NULL) {
		msUnitManager = bats::UnitManager::getInstance();
	}
}

void MedicAgent::computeActions()
{
	checkUnitsToHeal();
	computeMoveAction(true);
}

void MedicAgent::checkUnitsToHeal()
{
	try {
		int bestDist = INT_MAX;
		const Unit* toHeal = NULL;
		TilePosition closeSearchFrom = getUnit()->getTilePosition();

		/// @todo If assigned to squad, only heal units in the squad, or close units.
		/// Also heal close allied units if possible
		
		// Search for squad units
		if (getSquadId().isValid()) {
			bats::SquadCstPtr squad = msSquadManager->getSquad(getSquadId());
			closeSearchFrom = squad->getCenter();

			const vector<const UnitAgent*>& units = squad->getUnits();
			for (size_t i = 0; i < units.size(); ++i) {
				if (isMedicTarget(units[i]->getUnit())) {
					int distSquared = bats::getSquaredDistance(units[i]->getUnit()->getTilePosition(), getUnit()->getTilePosition());
					if (distSquared < bestDist) {
						bestDist = distSquared;
						toHeal = units[i]->getUnit();
					}
				}
			}
		}
		// Else - Search all our units
		else {
			const vector<UnitAgent*>& units = msUnitManager->getUnitsByFilter();
			for (size_t i = 0; i < units.size(); i++) {
				if (isMedicTarget(units[i]->getUnit())) {
					int distSquared = bats::getSquaredDistance(units[i]->getUnit()->getTilePosition(), getUnit()->getTilePosition());
					if (distSquared < bestDist &&
						distSquared <= bats::config::unit::medic::HEAL_SEARCH_DISTANCE_SQUARED)
					{
						bestDist = distSquared;
						toHeal = units[i]->getUnit();
					}	
				}
			}
		}

		// Check allied units too, if in squad it will search from the squad's center when checking
		// If the unit is close enough to the squad, otherwise it will use the unit's position
		const set<Player*>& players = Broodwar->allies();
		set<Player*>::const_iterator playerIt;
		for (playerIt = players.begin(); playerIt != players.end(); ++playerIt) {
			const set<Unit*>& units = (*playerIt)->getUnits();
			set<Unit*>::const_iterator unitIt;
			for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
				if (isMedicTarget(*unitIt)) {
					int distSquared = bats::getSquaredDistance((*unitIt)->getTilePosition(), getUnit()->getTilePosition());

					if (distSquared < bestDist &&
						bats::isWithinRange(closeSearchFrom, (*unitIt)->getTilePosition(), bats::config::unit::medic::HEAL_SEARCH_DISTANCE))
					{
						bestDist = distSquared;
						toHeal = (*unitIt);
					}
				}
			}
		}

		if (toHeal != NULL)
		{
			/// @todo remove const cast when fixed in BWAPI
			unit->useTech(TechTypes::Healing, const_cast<Unit*>(toHeal));
		}
	}
	catch(exception)
	{
		ERROR_MESSAGE(false, "[" << unit->getID() << "] checkUnitToHeal() error");
	}
}

bool MedicAgent::isMedicTarget(const Unit* unit)  const
{
	// Can only heal organic units
	if (!unit->getType().isOrganic())
	{
		return false;
	}

	// We can heal workers, but no point in following them
	if (unit->getType().isWorker())
	{
		return false;
	}

	// Don't heal loaded units, as in bunker or transport
	if (unit->isLoaded())
	{
		return false;
	}

	// Only heal damaged units
	if (unit->getHitPoints() == unit->getType().maxHitPoints()) {
		return false;
	}

	// Only heal alive units
	if (unit->getHitPoints() == 0) {
		return false;
	}

	// Only heal completed units
	if (!unit->isCompleted()) {
		return false;
	}

	// Unit shall exist
	if (!unit->exists()) {
		return false;
	}

	// Don't heal ourselves
	if (unit->getID() == getUnitID()) {
		return false;
	}

	return true;
}
