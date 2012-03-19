#pragma once

#include <vector>
#include <BWAPI/TilePosition.h>
#include "IdTypes.h"
#include "Utilities/KeyType.h"
#include "Utilities/KeyHandler.h"
#include "UnitSet.h"
#include "UnitComposition.h"
#include "UnitCompositionFactory.h"

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
	 * @param unitComposition the composition of units to use, if this is specified
	 * all units needs to be added directly in this constructor. Defaults to
	 * INVALID_UNIT_COMPOSITON.
	 */
	Squad(std::vector<UnitAgent*> units,
		bool disbandable = true,
		const UnitComposition& unitComposition = UnitCompositionFactory::INVALID_UNIT_COMPOSITION);

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
	 * Returns true if the squad is full, only applicable on squads with a unit composition
	 * @return true if the squad is full, false if not, and false if it does not have
	 * a valid unit composition.
	 */
	bool isFull() const;

	/**
	 * Adds a unit to the squad.
	 * @param pUnit the unit to add.
	 */
	void addUnit(UnitAgent* pUnit);

	/**
	 * Adds many units to the squad.
	 * @param units vector with units to add
	 */
	void addUnits(const std::vector<UnitAgent*>& units);

	/**
	 * Removes a unit from the squad. This should be called when a unit dies too.
	 * @param pUnit the unit to remove.
	 */
	void removeUnit(UnitAgent* pUnit);

	/**
	 * Returns the squad's id.
	 * @return squad's id.
	 */
	const SquadId& getSquadId() const;

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
	 * Sets the position to move to as a goal. This will reset all current goal positions.
	 * @param position the new goal position to move to.
	 * @see setGoalPositions() to set a list of positions, and reset current positions.
	 * @see addGoalPosition() to add one position to the back of the queue, without resetting.
	 * @see addGoalPositions() to add several position to the back of the queue, without resetting.
	 */
	void setGoalPosition(const BWAPI::TilePosition& position);

	/**
	 * Sets the positions to move to as goals. This will reset all current goal positions.
	 * @param positions the new goal positions to move to.
	 * @see setGoalPosition() to set one position, and reset current positions.
	 * @see addGoalPosition() to add one position to the back of the queue, without resetting.
	 * @see addGoalPositions() to add several position to the back of the queue, without resetting.
	 */
	void setGoalPositions(const std::list<BWAPI::TilePosition>& positions);

	/**
	 * Adds the position at the back of the queue.
	 * @param position the goal position to add at the back of the queue.
	 * @see setGoalPosition() to set one position, and reset current positions.
	 * @see setGoalPositions() to set a list of positions, and reset current positions.
	 * @see addGoalPositions() to add several position to the back of the queue, without resetting.
	 */
	void addGoalPosition(const BWAPI::TilePosition& position);

	/**
	 * Adds the positions at the back of the queue.
	 * @param positions the goal positions to add at the back of the queue.
	 * @see setGoalPosition() to set one position, and reset current positions.
	 * @see setGoalPositions() to set a list of positions, and reset current positions.
	 * @see addGoalPosition() to add one position to the back of the queue, without resetting.
	 */
	void addGoalPositions(const std::list<BWAPI::TilePosition>& positions);

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
	std::list<BWAPI::TilePosition> mGoalPositions;
	UnitComposition mUnitComposition;

	bool mDisbandable; /**< If the squad is allowed to be destroyed */
	bool mDisbanded;
	SquadStates mState;
	SquadId mId;

	static int mcsInstance; /**< Number of instances, used for init and release of KeyHandler. */
	static utilities::KeyHandler<_SquadType>* mpsKeyHandler;
};
}