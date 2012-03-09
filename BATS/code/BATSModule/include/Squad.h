#pragma once

#include <vector>
#include <BWAPI/TilePosition.h>
#include "Utilities/KeyType.h"
#include "Utilities/KeyHandler.h"
#include "UnitSet.h"

// Forward declarations
class UnitAgent;

// Namespace for the project
namespace bats {

/**
 * Base class for all BATS squads. Represents a squad of units with a shared goal.
 * The squad is composed of different unit types. 
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class Squad
{
public:
	/**
	 * Constructs a squad with the specified units, can set certain options of the squad.
	 * The default options actives the squad directly, the squad can be destroyed (and thus merged).
	 * @param units vector of all units that should be in the class.
	 * @param disbandable the squad can be destroyed, default is true
	 */
	Squad(std::vector<UnitAgent*> units,
		bool disbandable = true);

	/**
	 * Destructor, automatically disbands the squad if it hasn't been disbanded.
	 * This always succeeds disbanding the squad opposed of disband() which only tries.
	 */
	virtual ~Squad();

	/**
	 * Returns true if the squad has disbanded, thus it can safely be removed.
	 * @return true if the squad has disbanded.
	 */
	bool isDisbanded() const;

	/**
	 * Returns true if the squad can be disbanded.
	 * @return true if the squad can be disbanded.
	 */
	bool isDisbandable() const;

	/**
	 * Tries to disband the squad.
	 * @return true if the squad was successfully disbanded. Always fails and returns false if the
	 * squad isn't disbandable.
	 */
	virtual bool tryDisband();

	/**
	 * Computes general actions every frame. Calls computerSquadSpecificActions() if a goal exists
	 * and wasn't completed.
	 */
	void computeActions();

	/**
	 * Returns true if the squad is full, only applicable on needsFull squads
	 * @return true if the squad is full.
	 */
	bool isFull() const;

	/**
	 * States of the squad
	 */
	enum SquadStates {
		SquadState_First = 0,
		SquadState_Active = SquadState_First,
		SquadState_Inactive,
		SquadState_Lim
	};
	
protected:
	/**
	 * Virtual compute actions function. This function is called by computeActions().
	 * Implement this function to create specific squad behavior.
	 */
	virtual void computeSquadSpecificActions();

	/**
	 * Called when a goal fails. Disbands the squad, derive this function if you want to
	 * create a new repeating goal.
	 */
	virtual void onGoalFailed();

	/**
	 * Called when a goal succeeds. Disbands the squad, derive this function if you want to
	 * create a new goal repeating goal.
	 */
	virtual void onGoalSucceeded();

	/**
	 * Force disband on the squad
	 */
	void forceDisband();

	/**
	 * Sets the position to move to as a goal and an additional path to go via
	 * @param goalPosition the position the squad should move to
	 * @param via positions to go via to the goalPosition, optional.
	 */
	void moveTo(
		const BWAPI::TilePosition& goalPosition,
		const std::vector<BWAPI::TilePosition>& via = std::vector<BWAPI::TilePosition>());

private:
	/**
	 * Different goal states, should be moved later?
	 */
	enum GoalStates {
		GoalState_First = 0,
		GoalState_Success = GoalState_First,
		GoalState_Failed,
		GoalState_NotCompleted,
		GoalState_Lim
	};

	/**
	 * Called when a new goal should be created.
	 */
	virtual void createGoal() = 0;

	/**
	 * Checks whether the current goal is completed or not. The derived class
	 * handles whether or not the goal has been completed or not
	 * @return the current goal state
	 */
	virtual GoalStates getGoalState() const = 0;
	
	std::vector<UnitAgent*> mUnits;
	BWAPI::TilePosition mGoalPosition;
	std::vector<BWAPI::TilePosition> mMoveVia;

	bool mDisbandable; /**< If the squad is allowed to be destroyed */
	bool mDisbanded;
	SquadStates mState;
	utilities::KeyType<Squad> mId;

	static int mcsInstance; /**< Number of instances, used for init and release of KeyHandler. */
	static utilities::KeyHandler<Squad>* mpsKeyHandler;
};
}