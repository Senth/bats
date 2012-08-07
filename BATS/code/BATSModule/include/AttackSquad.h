#pragma once

#include "Squad.h"
#include "Typedefs.h"
#include <vector>

// Namespace for the project
namespace bats {

class AttackCoordinator;
class ExplorationManager;
class PlayerArmyManager;
class DefenseManager;

/**
 * Attacks a point on the map. The squad will intercept all enemy units on the way to
 * the point. If no point is specified it will choose a point it thinks is good for attacking.
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class AttackSquad : public Squad {
public:
	/**
	 * Constructor that takes units to be used with the squad.
	 * @param units all units to be added to the squad.
	 * @param distracting if the attack squad is distracting or not. Defaults to false
	 * @param unitComposition the unit composition the attack squad uses.
	 */
	AttackSquad(
		const std::vector<UnitAgent*>& units,
		bool distracting = false,
		const UnitComposition& unitComposition = UnitCompositionFactory::INVALID
	);

	/**
	 * Destructor
	 */
	virtual ~AttackSquad();

	/**
	 * Checks whether the attack squad is "in position". In position means the squad is
	 * on the last place before it will attack the goal. It will only go to this position
	 * if it waits for a WaitGoal to be finished.
	 * @return true if the Squad is currently at the waiting position before the goal. False
	 * if the Squad isn't there. True if it has no WaitGoal.
	 * @note This function will return true if it has no WaitGoal.
	 */
	bool isInPosition() const;

	/**
	 * Checks whether the squad is attacking, or rather if any of the squad's units are attacking.
	 * @return true if any unit of the squad is attacking, or under attack. False otherwise.
	 */
	bool isAttacking() const;

	/**
	 * Checks whether the attack squad is ready to attack the target goal. This tests whether
	 * isAttacking() or isInPosition() is true. Meaning it will be ready if it's in position or
	 * if it has already started attacking (which can be the case if the enemy meets it).
	 * @return true if the AttackSquad is ready to attack the target goal, else false.
	 */
	bool isReadyToAttack() const;

	/**
	 * Checks whether the AttackSquad is a distracting instead of a frontal attack.
	 * @return true if the attack is distracting, false if it is a frontal attack.
	 * @see isFrontalAttack()
	 */
	bool isDistracting() const;

	/**
	 * Checks whether the AttackSquad is a frontal attack instead of a frontal attack.
	 * @return true if the attack is a frontal attack, false if it is a distracting attack.
	 * @see isDistracting()
	 */
	bool isFrontalAttack() const;

	/**
	 * Returns true if the squad is following an allied squad
	 * @return true if the squad is following an allied squad
	 */
	bool isFollowingAlliedSquad() const;

	/**
	 * Returns the allied squad this squad is following
	 * @return allied squad this squad is following, NULL if not following any allied squad
	 */
	AlliedSquadCstPtr getAlliedSquad() const;

	/**
	 * Returns an AttackSquad shared_ptr instead of the original Squad shared_ptr.
	 * @note This function is not overridden, but more a helper function to return the
	 * right pointer for an attackSquad.
	 * @return shared_ptr to this AttackSquad
	 */
	AttackSquadPtr getThis() const;

	virtual std::string getName() const;
	virtual std::string getDebugInfo() const;

protected:
	/**
	 * Checks whether all enemy structures near the goal are dead. It will
	 * only return true if the goal is visible. It uses structures_destroyed_goal_distance
	 * for the radius. As of now it will not check the entire radius for buildings.
	 * @return true if all enemy structures near the goal are dead.
	 */
	bool isEnemyStructuresNearGoalDead() const;

	virtual void updateDerived();
	virtual void onWaitGoalAdded(const std::tr1::shared_ptr<WaitGoal>& newWaitGoal);
	virtual void onGoalSucceeded();
	virtual bool createGoal();
	virtual GoalStates checkGoalState() const;

	static AttackCoordinator* mpsAttackCoordinator;
	static ExplorationManager* mpsExplorationManager;
	static PlayerArmyManager* mpsPlayerArmyManager;
	static DefenseManager* mpsDefenseManager;
private:
	/**
	 * Check if the squad needs to regroup with the allied forces.
	 * @return true if the squad needs to regroup, always returns false if we're not
	 * following any allied squad.
	 */
	bool needsAlliedRegrouping() const;

	/**
	 * Checks if we're done with regrouping with the allied forces.
	 * @return true if squad finished regrouping with allied squad or if we're not
	 * following any allied squad.
	 */
	bool finishedAlliedRegrouping() const;

	/**
	 * Handle allied regrouping, checks whether the squads needs to regroup.
	 */
	void handleAlliedRegrouping();

	/**
	 * Checks if the squad is regrouping with the allied squad
	 * @return true if the squad is regrouping with the allied squad.
	 */
	bool isRegroupingWithAllied() const;

	/**
	 * Clears the allied regrouping, i.e. it will not try to regroup with the allied squad.
	 */
	void clearAlliedRegrouping();

	/**
	 * Checks if any unit in this squad targets a enemy structure.
	 * @return true if any unit targets a enemy structure.
	 */
	bool isTargetingEnemyStructure() const;

	/**
	 * Handles the logic of the attack squad when following an allied squad.
	 * Retreat functionality is handled in handleRetreat().
	 * This function only executes if we're actually following an allied squad.
	 * @see handleNormalBehavior()
	 */
	void handleFollowAllied();

	/**
	 * Handles the logic of the attack squad when we're NOT following an allied squad.
	 * Retreat functionality is handled in handleRetreat().
	 * This function only executes if we're NOT following an allied squad.
	 * @see handleFollowAllied()
	 */
	void handleNormalBehavior();

	/**
	 * Checks if the enemy is too strong to attack, and might order a disband of
	 * the squad—depends on if it follows an allied squad or if attacked alone.
	 * @note this shall not be confused with the Squad::handleRetreat() command.
	 */
	void handleRetreat();

	friend AttackCoordinator; // For setting goal position.

	bool mDistraction;	/**< If the attack is a distracting attack or not */
	bool mWaitInPosition;
	bool mAttackedEnemyStructures;
	AlliedSquadCstPtr mpAlliedSquadFollow;
};
}