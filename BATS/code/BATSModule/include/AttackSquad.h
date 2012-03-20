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

	void createGoal();
	
	Squad::GoalStates getGoalState() const;

protected:
	virtual void computeSquadSpecificActions();

private:

};
}