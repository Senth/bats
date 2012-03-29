#include "AttackCoordinator.h"
#include "SquadManager.h"
#include "Squad.h"
#include "AttackSquad.h"
#include "ExplorationManager.h"
#include "Config.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

using namespace bats;
using namespace std;
using std::tr1::shared_ptr;
using std::tr1::dynamic_pointer_cast;
using namespace BWAPI;

const double MAP_WIDTH_MAX = 256.0;
const double MAP_HEIGHT_MAX = 256.0;
const double MAP_DISTANCE_MAX_SQUARED = MAP_HEIGHT_MAX * MAP_HEIGHT_MAX + MAP_WIDTH_MAX * MAP_WIDTH_MAX;
const double MAP_DISTANCE_MAX = sqrt(MAP_DISTANCE_MAX_SQUARED);

AttackCoordinator* AttackCoordinator::mpsInstance = NULL;


// Helpers
/**
 * Returns true if the structure can upgrade, only checks structures for weapon/armor upgrades.
 * @return true if the structure can upgrade
 */
bool structureCanUpgrade(UnitType structure) {
	if (structure.getRace() == Races::Terran) {
		if (structure == UnitTypes::Terran_Armory ||
			structure == UnitTypes::Terran_Engineering_Bay)
		{
			return true;
		}
	} else if (structure.getRace() == Races::Zerg) {
		if (structure == UnitTypes::Zerg_Evolution_Chamber ||
			structure == UnitTypes::Zerg_Greater_Spire ||
			structure == UnitTypes::Zerg_Spire)
		{
			return true;
		}
	} else if (structure.getRace() == Races::Protoss) {
		if (structure == UnitTypes::Protoss_Forge ||
			structure == UnitTypes::Protoss_Cybernetics_Core)
		{
			return true;
		}
	}

	return false;
}

AttackCoordinator::AttackCoordinator() {
	mpSquadManager = NULL;
	mpExplorationManager = NULL;

	mpSquadManager = SquadManager::getInstance();
	mpExplorationManager = ExplorationManager::getInstance();
}

AttackCoordinator::~AttackCoordinator() {
	mpsInstance = NULL;
}

AttackCoordinator* AttackCoordinator::getInstance() {
	if (NULL == mpsInstance) {
		mpsInstance = new AttackCoordinator();
	}
	return mpsInstance;
}

#pragma warning(push) // REMOVE
#pragma warning(disable:4100) // REMOVE
void AttackCoordinator::requestAttack(const std::tr1::shared_ptr<Squad>& squad) {
	/// @todo
}

BWAPI::TilePosition AttackCoordinator::calculateAttackPosition(bool useDefendedWeight) const {
	WeightedPosition highestWeight(BWAPI::TilePositions::Invalid);


	// Get all expansions that haven't been checked for the last X seconds
	vector<BWAPI::TilePosition> notScoutedExpansions = mpExplorationManager->findNotCheckedExpansions();
	mpExplorationManager->removeOccupiedExpansions(notScoutedExpansions);

	// Add and calculate expansions
	for (size_t i = 0; i < notScoutedExpansions.size(); ++i) {
		WeightedPosition weightedPosition(notScoutedExpansions[i]);

		// Distance
		weightedPosition.distanceWeight = calculateDistanceWeight(weightedPosition.position);

		// Type
		weightedPosition.typeWeight = config::attack_coordinator::weights::EXPANSION_NOT_CHECKED;

		// Defended
		if (useDefendedWeight) {
			weightedPosition.defendedWeight = calculateDefendedWeight(weightedPosition.position);
		}

		// Total
		weightedPosition.calculateTotalWeight();

		// New highest weighted position
		if (highestWeight < weightedPosition) {
			highestWeight = weightedPosition;
		}
	}

	
	// Calculate weight for all spotted building positions
	const vector<shared_ptr<SpottedObject>>& spottedObjects = mpExplorationManager->getSpottedBuildings();
	for (size_t i = 0; i < spottedObjects.size(); ++i) {
		WeightedPosition weightedPosition(spottedObjects[i]->getTilePosition());
		
		// Distance
		weightedPosition.distanceWeight = calculateDistanceWeight(weightedPosition.position);

		// Type
		weightedPosition.typeWeight = calculateStructureTypeWeight(*spottedObjects[i]);

		// Defended
		if (useDefendedWeight) {
			weightedPosition.defendedWeight = calculateDefendedWeight(weightedPosition.position);
		}

		// Total
		weightedPosition.calculateTotalWeight();

		// New highest weighted position
		if (highestWeight < weightedPosition) {
			highestWeight = weightedPosition;
		}
	}


	/// @todo Implement Overlord functionality.


	return highestWeight.position;
}

double AttackCoordinator::calculateDistanceWeight(const BWAPI::TilePosition& attackPosition) const {
	double distanceWeight = 0.0;
	int cAttackSquads = 0;

	// Calculate the sum of distances from all attacking squads
	map<SquadId, shared_ptr<Squad>>::const_iterator squadIt;
	for (squadIt = mpSquadManager->begin(); squadIt != mpSquadManager->end(); ++squadIt) {
		shared_ptr<AttackSquad> attackSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);

		// It's an attack squad
		if (NULL != attackSquad) {
			DEBUG_MESSAGE_CONDITION(attackSquad->getGoal() == BWAPI::TilePositions::Invalid,
				utilities::LogLevel_Severe, "AttackCoordinator::calculateDistanceWeight() | " <<
				"AttackSquad doesn't have a valid goal!");
			assert(BWAPI::TilePositions::Invalid != attackSquad->getGoal());

			BWAPI::TilePosition diffPosition = attackPosition - attackSquad->getGoal();
			double distanceSquared =
				diffPosition.x() * diffPosition.x() +
				diffPosition.y() * diffPosition.y();

			distanceWeight += distanceSquared;
			++cAttackSquads;
		}
	}

	// Found attacking squads
	if (cAttackSquads > 0) {
		// Normalize by number of squads
		distanceWeight /= static_cast<double>(cAttackSquads);

		// Normalize by maximum map distance to get answer in [0.0,1.0]
		distanceWeight /= MAP_DISTANCE_MAX_SQUARED;
	}
	// No attacking squads, set distance weight to 1 to avoid it affecting others.
	else {
		distanceWeight = 1.0;
	}

	return distanceWeight;
}

using namespace config::attack_coordinator;

double AttackCoordinator::calculateStructureTypeWeight(const SpottedObject& structure) const {
	double weight = -1.0;
	const BWAPI::UnitType& structureType = structure.getType();

	// Expansion 
	if (structureType.isResourceDepot()) {
		weight = calculateExpansionTimeWeight(structure.getTilePosition());
	}
	// Addon
	else if (structureType.isAddon()) {
		weight = weights::ADDON_STRUCTURE;
	}
	// Supply building — cannot be expansion since we have already checked for that
	else if (structureType.supplyProvided() > 0) {
		weight = weights::SUPPLY_STRUCTURE;
	}
	// Upgrades
	else if (structureCanUpgrade(structureType)) {
		weight = weights::UPGRADE_STRUCTURE;
	}
	// Unit producing structures
	else if (structureType.canProduce()) {
		weight = weights::UNIT_PRODUCING_STRUCTURE;
	}
	// Other structures
	else {
		weight = weights::OTHER_STRUCTURE;
	}

	assert (weight != -1.0);
	return weight;
}

#pragma warning(push) // REMOVE
#pragma warning(disable:4100) // REMOVE
double AttackCoordinator::calculateDefendedWeight(const BWAPI::TilePosition& attackPosition) const {
	/// @todo implement calculateDefendedWeight
	return 1.0;
}
#pragma warning(pop) // REMOVE

double AttackCoordinator::calculateExpansionTimeWeight(const BWAPI::TilePosition& expansionPosition) const {
	/// @todo
	return 1.0;
}
#pragma warning(pop) // REMOVE