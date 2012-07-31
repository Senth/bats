#include "AttackCoordinator.h"
#include "SquadManager.h"
#include "Squad.h"
#include "AttackSquad.h"
#include "ExplorationManager.h"
#include "ResourceCounter.h"
#include "ResourceGroup.h"
#include "WaitGoalManager.h"
#include "WaitReadySquad.h"
#include "AlliedSquad.h"
#include "Config.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <memory.h>

using namespace bats;
using namespace std;
using std::tr1::shared_ptr;
using std::tr1::dynamic_pointer_cast;
using namespace BWAPI;
using namespace config::attack_coordinator;

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
	mpResourceCounter = NULL;
	mpWaitGoalManager = NULL;

	mpSquadManager = SquadManager::getInstance();
	mpExplorationManager = ExplorationManager::getInstance();
	mpResourceCounter = ResourceCounter::getInstance();
	mpWaitGoalManager = WaitGoalManager::getInstance();
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

void AttackCoordinator::requestAttack(AttackSquadRef squad) {
	// Special case when squad is frontal attack and player squad has attacking big
	if (squad->isFrontalAttack() && squad->isFollowingAlliedSquad()) {
		AlliedSquadCstPtr alliedSquad = squad->getAlliedSquad();
		const TilePosition& alliedCenter = alliedSquad->getCenter();
		if (TilePositions::Invalid != alliedCenter) {
			// Find closest attack position to allied squad
			std::pair<TilePosition, int> foundAttackPosition = mpExplorationManager->getClosestSpottedBuilding(alliedCenter);

			TilePosition attackTarget = TilePositions::Invalid;

			// Found position
			if (TilePositions::Invalid != foundAttackPosition.first) {
				attackTarget = foundAttackPosition.first;
			}
			// Else use squad attack target
			else {
				const TilePosition alliedTarget = alliedSquad->getTargetPosition();
				if (TilePositions::Invalid != alliedTarget) {
					attackTarget = alliedTarget;
				} else {
					attackTarget = alliedCenter;
				}
			}

			if (TilePositions::Invalid != attackTarget) {
				squad->setGoalPosition(attackTarget);
			}
		}

		return;
	}


	// Attack position
	TilePosition bestAttackPosition = calculateAttackPosition(squad->isDistracting(), squad);
	squad->setGoalPosition(bestAttackPosition);


	// Add existing wait goals to new squad
	WaitGoalIterators waitGoalIterators;

	waitGoalIterators = mpWaitGoalManager->getWaitGoalsBySet(config::wait_goals::ATTACK_COORDINATION);

	multimap<string, shared_ptr<WaitGoal>>::const_iterator waitGoalIt;
	for (waitGoalIt = waitGoalIterators.first; waitGoalIt != waitGoalIterators.second; ++waitGoalIt) {
		squad->addWaitGoal(waitGoalIt->second);
	}

	
	// Create new wait goal for other squads
	shared_ptr<WaitGoal> newWaitGoal = shared_ptr<WaitGoal>(
		new WaitReadySquad(squad, config::attack_coordinator::WAIT_GOAL_TIMEOUT)
	);
	mpWaitGoalManager->addWaitGoal(newWaitGoal);


	// Add new wait goal to existing squads
	map<SquadId, shared_ptr<Squad>>::iterator squadIt;
	for (squadIt = mpSquadManager->begin(); squadIt != mpSquadManager->end(); ++squadIt) {
		shared_ptr<AttackSquad> currentAttackSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);

		if (NULL != currentAttackSquad &&
			currentAttackSquad != squad &&
			currentAttackSquad->getState() == Squad::State_Active)
		{
			currentAttackSquad->addWaitGoal(newWaitGoal);
		}
	}
}

BWAPI::TilePosition AttackCoordinator::calculateAttackPosition(bool useDefendedWeight, AttackSquadRef squad) const {
	WeightedPosition highestWeight(BWAPI::TilePositions::Invalid);

	highestWeight.distanceWeight = 0.0;
	highestWeight.calculateTotalWeight();


	// Get all expansions that haven't been checked for the last X seconds
	vector<BWAPI::TilePosition> notScoutedExpansions = mpExplorationManager->findNotCheckedExpansions();
	mpExplorationManager->removeOccupiedExpansions(notScoutedExpansions);

	// Add and calculate expansions
	for (size_t i = 0; i < notScoutedExpansions.size(); ++i) {
		WeightedPosition weightedPosition(notScoutedExpansions[i]);

		// Distance
		weightedPosition.distanceWeight = calculateDistanceWeight(weightedPosition.position, squad);

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
		weightedPosition.distanceWeight = calculateDistanceWeight(weightedPosition.position, squad);

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

double AttackCoordinator::calculateDistanceWeight(const BWAPI::TilePosition& attackPosition, AttackSquadRef squad) const {
	double distanceWeight = 0.0;
	int cAttackSquads = 0;

	// Calculate the sum of distances from all attacking squads
	map<SquadId, shared_ptr<Squad>>::const_iterator squadIt;
	for (squadIt = mpSquadManager->begin(); squadIt != mpSquadManager->end(); ++squadIt) {
		shared_ptr<AttackSquad> attackSquad = dynamic_pointer_cast<AttackSquad>(squadIt->second);

		// It's not this squad and it's active
		if (NULL != attackSquad && attackSquad != squad && attackSquad->getState() == Squad::State_Active) {
			DEBUG_MESSAGE_CONDITION(attackSquad->getGoalPosition() == BWAPI::TilePositions::Invalid,
				utilities::LogLevel_Severe, "AttackCoordinator::calculateDistanceWeight() | " <<
				"AttackSquad doesn't have a valid goal!");
			assert(BWAPI::TilePositions::Invalid != attackSquad->getGoalPosition());

			BWAPI::TilePosition diffPosition = attackPosition - attackSquad->getGoalPosition();
			double distanceSquared =
				diffPosition.x() * diffPosition.x() +
				diffPosition.y() * diffPosition.y();

			distanceWeight += distanceSquared;
			++cAttackSquads;
		}
	}

	
	/// @todo check where player is attacking


	// Found attacking squads
	if (cAttackSquads > 0) {
		// Normalize by number of squads
		distanceWeight /= static_cast<double>(cAttackSquads);

		// Normalize by maximum map distance to get answer in [0.0,1.0]
		distanceWeight /= MAP_DISTANCE_MAX_SQUARED;
	}
	// No attacking squads, set distance weight to 1 to avoid it affecting other weights
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
	double weight = 1.0;

	shared_ptr<const ResourceGroup> resourceGroup = mpResourceCounter->getResourceGroup(expansionPosition);

	if (NULL != resourceGroup) {
		// Calculate how much the interval is
		double intervalLength = weights::EXPANSION_MAX;
		if (false == weights::EXPANSION_CEIL) {
			intervalLength -= weights::EXPANSION_MIN;
		}

		weight = resourceGroup->getResourcesLeftInFraction();
		weight *= intervalLength;

		// Shall we ceil or add it?
		if (weights::EXPANSION_CEIL) {
			if (weight < weights::EXPANSION_MIN) {
				weight = weights::EXPANSION_MIN;
			}
		} else {
			weight += weights::EXPANSION_MIN;
		}
	} else {
		ERROR_MESSAGE(false, "Could not find Resource group for expansion position: (" <<
			expansionPosition.x() << ", " << expansionPosition.y() << ")!"
		);
	}

	return weight;
}