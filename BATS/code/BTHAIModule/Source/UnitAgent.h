#ifndef __UNITAGENT_H__
#define __UNITAGENT_H__

#include <BWAPI.h>
#include "BaseAgent.h"

#define DISABLE_UNIT_AI 0

class PFManager;

namespace bats {
	class SquadManager;
}

/** The UnitAgent is the base agent class for all agents handling mobile units. If a unit is created and no
 * specific agent for that type is found, the unit is assigned to a UnitAgents. UnitAgents can attack and
 * assist building under enemy fire, but cannot use any special abilities. To use abilities such as Terran Vultures
 * dropping spider mines, an agent implementation for that unit type must be created.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (mattues.magnusso@gmail.com)
 * Implemented retreat functionality
 * and moved some common calculations from the units to UnitAgent.
 */
class UnitAgent : public BaseAgent {	
public:
	UnitAgent(BWAPI::Unit* mUnit);

	int dropped;

	/** Called each update to issue orders. */
	void computeActions();

	/** Used in debug modes to show a line to the agents' goal. */
	void debug_showGoal();

	/** Handles actions for kiting agents. */
	void computeKitingActions();

	/**
	 * Finds an target for the unit to attack. But only attacks if the unit isn't in
	 * a squad that avoids enemies.
	 */
	void findAndTryAttack();

	/**
	 * Handles attacking unit actions, this function takes into account if the squad 
	 * is currently retreating to make the unit defensive.
	 * @see computeMoveActions(bool,bool) if you want to override the squad's state.
	 */
	void computeMoveAction();

	/**
	 * Handles attacking unit actions. Either sets the unit as defensive or not and
	 * does not take into account the squad's state.
	 * @param defensive set to true if the unit shall avoid all enemies
	 * @param forceMove set to true to force the unit to move, defaults to false
	 * @see computeMoveActions() if you want to use the squad's behavior.
	 */
	void computeMoveAction(bool defensive, bool forceMove = false);

	/**
	 * Returns true if the unit is an air unit
	 * @return true if the unit travels by air
	 */
	bool isAir() const;

	/**
	 * Returns true if the unit is a ground unit.
	 * @return true if the unit travels by ground.
	 */
	bool isGround() const;

	/**
	 * Returns true if the unit can transport ground units.
	 * @return true if the unit can transport ground units.
	 */
	bool isTransport() const;

	/** Returns the number of own units that are within 6 tiles range of the agent. */
	int friendlyUnitsWithinRange() const;

	/** Returns the number of own units that are within maxRange of the agent. */
	int friendlyUnitsWithinRange(int maxRange) const;

	/** Returns the number of own units that are within maxRange of the specified tile. */
	int friendlyUnitsWithinRange(BWAPI::TilePosition tilePos, int maxRange) const;

	/** Returns the number of enemy units and buildings that can attack and are within firerange of the agent. */
	int enemyAttackingUnitsWithinRange() const;

	/** Returns the number of enemy units and buildings that can attack and are within range of the center tile. */
	int enemyAttackingUnitsWithinRange(int maxRange, BWAPI::TilePosition center) const;

	/** Returns the number of enemy units and buildings that can attack and are within firerange of the specified unit type. */
	int enemyAttackingUnitsWithinRange(BWAPI::UnitType type) const;

	/** Returns the number of enemy units and buildings that are within maxRange of the agent. */
	int enemyUnitsWithinRange(int maxRange) const;

	/** Returns the number of enemy ground units and buildings that are within maxRange of the agent. */
	int enemyGroundUnitsWithinRange(int maxRange) const;

	/** Calculates the number of enemy sieged Siege Tanks within Siege Tank range. */
	int enemySiegedTanksWithinRange(BWAPI::TilePosition center) const;

	/** Returns the number of enemy ground units and buildings that are within maxRange of the center tile, and can attack the agent. */
	int enemyGroundAttackingUnitsWithinRange(BWAPI::TilePosition center, int maxRange) const;

	/** Returns the number of flying units that are within maxRange of the agent. */
	int enemyAirUnitsWithinRange(int maxRange) const;

	/** Checks if a unit can be in defensive mode. This happens if 1) the unit is in cooldown and 2) there are opponent
	 * units that can attack it. */
	bool useDefensiveMode() const;

	/** Returns the number of flying units that can target ground and are within maxRange of the agent. */
	int enemyAirToGroundUnitsWithinRange(int maxRange) const;

	/** Returns the number of flying unit that are within maxRange of the center tile, and can attack the agent. */
	int enemyAirAttackingUnitsWithinRange(BWAPI::TilePosition center, int maxRange) const;

	/** Returns the max firerange of the ground weapon this agent has, or -1 if it cannot attack ground. */
	int getGroundRange() const;

	/** Returns the max firerange of the ground weapon a unit of the specified type has, or -1 if it cannot attack ground. */
	static int getGroundRange(BWAPI::UnitType type);

	/** Returns true if the attacker UnitType can attack target UnitType. Note: Does not take spells into account, only weapons. */
	static bool canAttack(BWAPI::UnitType attacker, BWAPI::UnitType target);

	/** Returns the max firerange of the air weapon this agent has, or -1 if it cannot attack air. */
	int getAirRange() const;

	/** Returns the max firerange of the ground weapon a unit of the specified type has, or -1 if it cannot attack air. */
	static int getAirRange(BWAPI::UnitType type);
	
	/** Returns the closest organic enemy unit within maxRange, or NULL if not found. */
	BWAPI::Unit* getClosestOrganicEnemy(int maxRange);

	/** Returns the closest enemy unit that is shielded and within maxRange, or NULL if not found. */
	BWAPI::Unit* getClosestShieldedEnemy(int maxRange);

	/** Returns the closest enemy turret or other attacking building within maxRange, or NULL if not found. */
	BWAPI::Unit* getClosestEnemyTurret(int maxRange);

	/** Returns the closest enemy turret, other attacking building or mechanical unit within maxRange that
	 * can attack air units, or NULL if not found. */
	BWAPI::Unit* getClosestEnemyAirDefense(int maxRange);

	/** Orders a Protoss unit to recharge shields. */
	bool chargeShields();

	/** Used to print info about this agent to the screen. */
	void printInfo() const;

protected:
	static bats::SquadManager* mpsSquadManager;

private:
	static PFManager* mpsPfManager;
};

#endif
