#pragma once

#include <memory.h>
#include <BWAPI/TilePosition.h>
#include <BWAPI/UnitType.h>
#include "BTHAIModule/Source/SpottedObject.h"
#include "SquadDefs.h"

// Namespace for the project
namespace bats {

// Forward declarations
class ExplorationManager;
class SquadManager;
class AttackSquad;
class ResourceCounter;
class WaitGoalManager;

/**
 * Coordinates attack between squads making sure they attack at the same time and
 * attacking at different places.
 * 
 * A squad can request where to attack, the AttackCoordinator will then calculate
 * a place where to attack, depending on the type of attack (big frontal, or drop)
 * if there's any current attack issued, it will then prioritize locations further
 * away from that attack. It takes into account the teammate player's attack, if the
 * teammate is doing a big attack and itself is a big attack it will always
 * join the attack, otherwise it will simply treat it as just another attack.
 * 
 * The priority list: \n
 * <ol>
 * <li>Not scouted expansion</li>
 * <li>Expansions, weight is dynamic, depends on how fresh the expansion is</li>
 * <li>Addons</li>
 * <li>Supplies</li>
 * <li>Upgrades</li>
 * <li>Unit producing structures</li>
 * <li>Other structures</li>
 * </ol>
 * 
 * 
 * @todo Can be used as a tip giver.
 * 
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class AttackCoordinator {
public:
	/**
	 * Destructor
	 */
	virtual ~AttackCoordinator();

	/**
	 * Returns the instance of AttackCoordinator.
	 * @return instance of AttackCoordinator.
	 */
	static AttackCoordinator* getInstance();

	/**
	 * A squad can request an attack. The squad will then get a (new) goal position
	 * to attack. If there exist other squads, it will get a WaitGoal to wait for the
	 * other squads. Existing squads will add a WaitGoal for the specified Squad to
	 * get ready (Squad::isReady()).
	 * @param[in,out] squad the squad that requests an attack
	 */
	void requestAttack(AttackSquadRef squad);

private:
	/**
	 * Singleton constructor to enforce singleton usage.
	 */
	AttackCoordinator();

	/**
	 * Calculates and returns the highest priority position depending on the state of the
	 * game.
	 * @param useDefendedWeight if the calculation shall include defended weight.
	 * @param squad the squad we want to calculate the attack position for
	 * @return a TilePosition with the highest priority for attacking.
	 */
	BWAPI::TilePosition calculateAttackPosition(bool useDefendedWeight, AttackSquadRef squad) const;

	/**
	 * Calculates the distance weight depending on other attack squad positions. The
	 * further away the greater the weight. If more two or more squads exist it will
	 * normalize the weight by sum the weight and divide it by the squads.
	 * The weight is an exponential function.
	 * 
	 * @param attackPosition where to calculate the weight.
	 * @param squad the squad we want to calculate the weight for
	 * @return normalized distance weight [0.0,1.0]
	 */
	double calculateDistanceWeight(const BWAPI::TilePosition& attackPosition, AttackSquadRef squad) const;

	/**
	 * Calculates the weight depending on the structure type.
	 * @param structure the structure as a spotted object.
	 * @return normalized weight [0.0,1.0]
	 */
	double calculateStructureTypeWeight(const SpottedObject& structure) const;

	/**
	 * Calculates the weight depending on how well defended a position is.
	 * This function uses a radius from the attack position. Radius is defined in
	 * the config file. It will always include structures that can attack (missile turrets)
	 * and units if they are visible.
	 * 
	 * Shall only be used with distracting attack since it does not take our army into account
	 * it will only try to avoid attack positions with units.
	 * @param attackPosition where to calculate the weight.
	 * @return normalized weight [0.0,1.0]
	 */
	double calculateDefendedWeight(const BWAPI::TilePosition& attackPosition) const;

	/**
	 * Calculates the weight of the expansion. This weight decreases by the amount of minerals
	 * that has been mined from the expansion. I.e. an newly created expansion will have
	 * a high weight.
	 * 
	 * The weight decrement is linear by the number of minerals left. The weight is normalized
	 * between [0.0,1.0]. You can in, however, override the minimum weight in config.ini.
	 * Note that the if you use expansion_time_min = 0.2 it will ceil 
	 * expansions with less than 20% of mineral fields to a weight of 0.2. If you do not want
	 * to ceil the value but instead treat 20% as 20% from 0.2 to 1.0 (i.e. 0.36) you
	 * have to disable the ceil function with expansion_time_ceil = false
	 */
	double calculateExpansionTimeWeight(const BWAPI::TilePosition& expansionPosition) const;

	/**
	 * A weighted position
	 */
	struct WeightedPosition {
		double defendedWeight;
		double typeWeight;
		double distanceWeight;
		BWAPI::TilePosition position;

		/**
		 * Default constructor sets the weights to 1. Takes an optional position
		 * @param position the position of the weighted position
		 */
		WeightedPosition(const BWAPI::TilePosition& position = BWAPI::TilePositions::Invalid) :
			defendedWeight(1.0),
			typeWeight(1.0),
			distanceWeight(1.0),
			position(position),
			mTotalWeight(-1.0)
		{}

		/**
		 * Calculates the total weight of the position.
		 */
		void calculateTotalWeight() {
			mTotalWeight = defendedWeight * typeWeight * distanceWeight;
		}

		/**
		 * Returns the total weight as a product of defended, type, and distance weight.
		 * @return total weight, negative if the weight wasn't calculated.
		 */
		inline double getTotalWeight() const {
			return mTotalWeight;
		}

		/**
		 * Operator for sorting by the total weight.
		 * @param right the other WeightedPosition to check with
		 * @return true if this (left) WeightedPosition is smaller than the other
		 * (right) WeightedPosition
		 */
		inline bool operator<(const WeightedPosition& right) {
			return mTotalWeight < right.mTotalWeight;
		}

	private:
		double mTotalWeight;
	};

	SquadManager* mpSquadManager;
	ExplorationManager* mpExplorationManager;
	ResourceCounter* mpResourceCounter;
	WaitGoalManager* mpWaitGoalManager;

	static AttackCoordinator* mpsInstance;
};
}