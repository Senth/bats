#pragma once

#include "Squad.h"
#include <vector>

// Namespace for the project
namespace bats {

class AttackCoordinator;
class ExplorationManager;

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
	 */
	AttackSquad(const std::vector<UnitAgent*> units, bool distracting = false);

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
	 * Checks whether the AttackSquad is a distracting or frontal attack.
	 * @return true if the attack is distracting, false if it is a frontal attack.
	 */
	bool isDistracting() const;

	/**
	 * Returns an AttackSquad shared_ptr instead of the original Squad shared_ptr.
	 * @note This function is not overridden, but more a helper function to return the
	 * right pointer for an attackSquad.
	 * @return shared_ptr to this AttackSquad
	 */
	std::tr1::shared_ptr<AttackSquad> getThis() const;

protected:
	virtual void computeSquadSpecificActions();
	virtual void onWaitGoalAdded(const std::tr1::shared_ptr<WaitGoal>& newWaitGoal);
	virtual bool createGoal();
	virtual GoalStates checkGoalState() const;

	static AttackCoordinator* mpsAttackCoordinator;
	static ExplorationManager* mpsExplorationManager;
private:
	bool mDistraction;	/**< If the attack is a distracting attack or not */
	bool mWaitInPosition;
};
}