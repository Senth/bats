#pragma once

#include "Squad.h"
#include <vector>

// Namespace for the project
namespace bats {

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
	 */
	AttackSquad(const std::vector<UnitAgent*> units);

	/**
	 * Destructor
	 */
	virtual ~AttackSquad();

	virtual bool createGoal();

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

protected:
	virtual void computeSquadSpecificActions();

private:

};
}