#pragma once

#include "Squad.h"
namespace bats{

/**
 * 
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 */
class ScoutSquad : public Squad{
	public:
		/**
		 * Constructor that takes units to be used with the squad.
		 * @param units all units to be added to the squad.
		 * @param avoidEnemy avoid enemy if spotted and explores remaining part of the map,
		 * defaults to true
		 * @param unitComposition the unit composition to use for the squad.
		 */
		ScoutSquad(const std::vector<UnitAgent*> units, 
			bool avoidEnemy = true, 
			const UnitComposition& unitComposition = UnitCompositionFactory::INVALID);
		
		/**
		 * Destructor
		 */
		virtual ~ScoutSquad();
		
	protected:
		virtual void updateDerived();
		virtual bool createGoal();
		virtual Squad::GoalStates checkGoalState() const;
		virtual void onGoalFailed();
		virtual void onGoalSucceeded();
		virtual std::string getName() const;

	private:
		bool mAvoidEnemy;	/**< If the squad avoids enemy or not */
};
}