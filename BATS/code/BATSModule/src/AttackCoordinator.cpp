#include "AttackCoordinator.h"
#include "SquadManager.h"
#include "Squad.h"
#include "AttackSquad.h"
#include "ExplorationManager.h"
#include "Config.h"
#include <cstdlib>
#include <cmath>

using namespace bats;
using namespace std;
using std::tr1::shared_ptr;
using std::tr1::dynamic_pointer_cast;

const double MAP_WIDTH_MAX = 256.0;
const double MAP_HEIGHT_MAX = 256.0;
const double MAP_DISTANCE_MAX_SQUARED = MAP_HEIGHT_MAX * MAP_HEIGHT_MAX + MAP_WIDTH_MAX * MAP_WIDTH_MAX;
const double MAP_DISTANCE_MAX = sqrt(MAP_DISTANCE_MAX_SQUARED);

AttackCoordinator* AttackCoordinator::mpsInstance = NULL;

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

#pragma warning(push)	// REMOVE
#pragma warning(disable:4100) // REMOVE
void AttackCoordinator::requestAttack(const std::tr1::shared_ptr<Squad>& squad) {
	/// @todo
}
#pragma warning(pop) // REMOVE

BWAPI::TilePosition AttackCoordinator::calculateAttackPosition() const {
	/// @todo
	return BWAPI::TilePositions::Invalid;
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

double AttackCoordinator::calculateStructureTypeWeight(const SpottedObject& structure) const {
	double weight = -1.0;
	const BWAPI::UnitType& structureType = structure.getType();

	// Expansion 
	if (structureType.isResourceDepot()) {
		weight = calculateExpansionTimeWeight(structure.getTilePosition());
	}
	// Addon
	else if (structureType.isAddon()) {
		weight = 2.0;
	}
	else if (structureType.isAddon()) {
		
	}

	assert (weight != -1.0);
	return weight;
}

#pragma warning(push)	// REMOVE
#pragma warning(disable:4100) // REMOVE
double AttackCoordinator::calculateDefendedWeight(const BWAPI::TilePosition& attackPosition) const {
	/// @todo
	return 1.0;
}

double AttackCoordinator::calculateExpansionTimeWeight(const BWAPI::TilePosition& expansionPosition) const {
	/// @todo
	return 1.0;
}
#pragma warning(pop) // REMOVE