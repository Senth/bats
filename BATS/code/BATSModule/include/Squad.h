#pragma once

#include <vector>
#include <memory.h>
#include <BWAPI/TilePosition.h>
#include "IdTypes.h"
#include "Utilities/KeyType.h"
#include "Utilities/KeyHandler.h"
#include "UnitSet.h"
#include "UnitComposition.h"
#include "UnitCompositionFactory.h"
#include "WaitGoal.h"

// Forward declarations
class UnitAgent;

// Namespace for the project
namespace bats {

class SquadManager;

/**
 * Base class for all BATS squads. Represents a squad of units with a shared goal.
 * The squad is composed of different unit types. The squad must always be activated using
 * activate() function, else it won't do anything.
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class Squad
{
public:
	/**
	 * Constructs a squad with the specified units, can set certain options of the squad.
	 * The default options actives the squad directly, the squad can be destroyed (and thus merged).
	 * @param units vector of all units that should be in the class.
	 * @param avoidEnemyUnits if the squad shall avoid enemy units at all costs. Defaults to false.
	 * @param disbandable the squad can be destroyed, default is true
	 * @param unitComposition the composition of units to use, if this is specified
	 * all units needs to be added directly in this constructor. Defaults to
	 * INVALID_UNIT_COMPOSITON.
	 */
	Squad(std::vector<UnitAgent*> units,
		bool avoidEnemyUnits = false,
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
	 * Returns true if the squad is currently regrouping
	 * @return true if the squad is regrouping
	 */
	bool isRegrouping() const;

	/**
	 * Returns true if the squad is empty
	 * @return true if the squad is empty
	 */
	bool isEmpty() const;

	/**
	 * Deactivates the squad. This will cause the squad to do nothing. Also
	 * the initial state of the squad.
	 */
	void deactivate();

	/**
	 * Activates the squad. This will at least process the squad, depending on if
	 * it needs a unit composition or not it might not do anything...
	 */
	void activate();

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
	 * Returns the center of the squad.
	 * @return center of the squad.
	 * @note Can be very wrong if it newly added units from all over the map to the squad.
	 * Because it will then use the median of all units.
	 */
	BWAPI::TilePosition getCenter() const;

	/**
	 * Calculates the distance to the unit furthest away from the center of the squad.
	 * Because this function is calculation heavy it will cache the distance for 
	 * CALC_FURTHEST_AWAY_TIME (calc_furthest_away_time in config file). You can override
	 * this setting by setting the parameter forceRecalculate to true, then it will always 
	 * recalculate the distance.
	 * @param forceRecalculate set this to true if you want to force recalculation, thus
	 * being sure you get the right answer, defaults to false.
	 * @return the distance to the unit furthest aways from the center of the squad. If no
	 * units exist it will return 0.0.
	 */
	double getFurthestUnitAwayDistance(bool forceRecalculate = false) const;


	/**
	 * Returns true if the squad is full, only applicable on squads with a unit composition
	 * @return true if the squad is full, false if not, and false if it does not have
	 * a valid unit composition.
	 */
	bool isFull() const;

	/**
	 * Returns true if this squad travels by ground. This squad can have flying units.
	 * It just means that some units travels have to travel by ground.
	 * @return true if this squad travels by ground.
	 * @see The direct opposite of travelsByAir()
	 */
	bool travelsByGround() const;

	/**
	 * Returns true if this squad travels by air. This squad can have ground units,
	 * but these are always loaded into a transportation ship when traveling. Meaning
	 * that the squad can reach all places. 
	 * @return True if the squad travels by air. Will return false if some units aren't
	 * meant to get loaded into the transportation ship. Will also return false if there
	 * aren't enough of transportation ships to load all ground units.
	 */
	bool travelsByAir() const;

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
	 * Adds a WaitGoal to the Squad. While this doesn't do anything in itself the derived
	 * Squad can implement a feature that waits for this WaitGoal to complete before it
	 * executes an option. AttackSquad and DropSquad uses this feature together with
	 * WaitReadySquad to coordinate an attack to start attacking on different places at
	 * the same time. This function will call onWaitGoalAdded() that can be overloaded in
	 * the derived classes.
	 * @param waitGoal the goal to add.
	 */
	void addWaitGoal(const std::tr1::shared_ptr<WaitGoal>& waitGoal);

	/**
	* Adds several WaitGoal(s) to the Squad.
	* Will call onWaitGoalAdded() for each WaitGoal in the vector.
	* @param waitGoals all WaitGoal(s) to add.
	* @see addWaitGoal() for a more detailed description.
	*/
	void addWaitGoals(const std::vector<std::tr1::shared_ptr<WaitGoal>>& waitGoals);

	/**
	 * Checks whether the squad has wait goals or not. Can mean no wait goals were added
	 * or all wait goals were are done, either successfully or failed by some means.
	 * @return true if the squad has no wait goals. 
	 */
	bool hasWaitGoals() const;

	/**
	 * Returns the squad's id.
	 * @return squad's id.
	 */
	const SquadId& getSquadId() const;

	/**
	 * Returns the current goal position. If the squad has several goal positions
	 * it will only return the first one.
	 * @return current goal position.  If the squad has no goal it will return
	 * BWAPI::TilePositions::Invalid.
	 */
	const BWAPI::TilePosition& getGoal() const;

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

	/**
	 * Returns all the squad's units
	 * @return all squad's units
	 */
	const std::vector<UnitAgent*> getUnits() const;

	/**
	 * Returns this pointer to the squad as an shared_ptr instead of just this
	 * @return shared_ptr to this squad
	 */
	std::tr1::shared_ptr<Squad> getThis() const;

	/**
	 * States of the squad
	 */
	enum States {
		State_First = 0,
		State_Active = State_First,
		State_Initializing,
		State_Inactive,
		State_Lim
	};

	/**
	 * Returns the current state of the Squad
	 * @return current state of the Squad
	 */
	States getState() const;

	/**
	 * Returns true if the squad is close to the specified position. Will always
	 * return false if the squad is regrouping, because when regrouping the center might
	 * not be valid at all, if 4 units are at the top left of the map and 4 units are
	 * at bottom right of the map the center will be in the center of the map, but no units
	 * are really close to the center. Uses the default range set in
	 * config::squad::CLOSE_DISTANCE_RANGE (close_distance_range in config file).
	 * @note Takes the type of squad into account, if the squad travelsByAir() it will use
	 * the direct distance, else it will use ground distance.
	 * @param position the position to test if the squad is close to.
	 * @return true if the center position of the squad is close to that position. Returns
	 * false if the center position isn't in range or if the squad is regrouping.
	 * @see isCloseTo(const BWAPI::TilePosition&,double) for you own radius.
	 */
	bool isCloseTo(const BWAPI::TilePosition& position) const;

	/**
	 * Returns true if the squad is close to the specified position. Will always
	 * return false if the squad is regrouping, because when regrouping the center might
	 * not be valid at all, if 4 units are at the top left of the map and 4 units are
	 * at bottom right of the map the center will be in the center of the map, but no units
	 * are really close to the center.
	 * @note Takes the type of squad into account, if the squad travelsByAir() it will use
	 * the direct distance, else it will use ground distance.
	 * @param position the position to test if the squad is close to.
	 * @param range the distance between position and the center
	 * of the squad needs to be less or equal to the this range.
	 * @return true if the center position of the squad is close to that position. Returns
	 * false if the center position isn't in range or if the squad is regrouping.
	 * @see isCloseTo(const BWAPI::TilePosition&) for using the default radius.
	 */
	bool isCloseTo(const BWAPI::TilePosition& position, double range) const;
	
protected:
	/**
	 * Different goal states.
	 */
	enum GoalStates {
		GoalState_First = 0,
		GoalState_Succeeded = GoalState_First,
		GoalState_Failed,
		GoalState_NotCompleted,
		GoalState_Lim
	};

	/**
	 * Virtual compute actions function. This function is called by computeActions().
	 * Implement this function to create specific squad behavior.
	 */
	virtual void computeSquadSpecificActions();

	/**
	 * Check whether the goal is completed or not. Shall return whether
	 * the current goal state of the squad.
	 * @return current goal state of the squad.
	 */
	virtual GoalStates checkGoalState() const = 0;

	/**
	 * Called when a new goal shall be created.
	 * @returns true a goal was successfully created.
	 */
	virtual bool createGoal() = 0;

	/**
	 * Sets whether the squad shall mainly be used for air transportation.
	 * @param usesAir set this to true if the squad shall mainly be used for air transportation.
	 * False if it not only uses air transportation.
	 * @note This does not assure that the squad always will use air transportation. It is merely
	 * used for calculating if it can travel by air or not. If it is set to false, it will never
	 * try to check if it can travel by air or not.
	 * @see travelsByAir() for how it is calculated if it travelsy by air or not.
	 */
	void setAirTransportation(bool usesAir);

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
	 * Called when a new WaitGoal has been added. Does nothing in the base class Squad.
	 * @param newWaitGoal the new WaitGoal that has been added. This is the same goal as
	 * the last goal in the wait goal vector.
	 */
	virtual void onWaitGoalAdded(const std::tr1::shared_ptr<WaitGoal>& newWaitGoal);

	/**
	 * Called when a wait goal has finished. This can be either by success or failure.
	 * The reason of why it finished can be returned by WaitGoal::getWaitState(). Does
	 * nothing in the base class Squad.
	 * @param finishedWaitGoal the wait goal that has recently finished. This goal is
	 * no longer in the wait goal vector.
	 */
	virtual void onWaitGoalFinished(const std::tr1::shared_ptr<WaitGoal>& finishedWaitGoal);

	/**
	 * Force disband on the squad
	 */
	void forceDisband();

	/**
	 * Update the current goal on all the squad's units to the current goal in the list.
	 * If no goal exists, set an invalid goal. This also removes any temporary goal positions.
	 */
	void updateUnitGoals();

	/**
	 * Overrides the squad's units' goal with a temporary goal position. Can be used to wait in a
	 * certain position before attacking. To go back to the main goal, call updateUnitGoals().
	 * @param temporaryPosition the new temporary goal position of the units. Nothing happens
	 * if the temporary goal is set to BWAPI::TilePositions::Invalid.
	 */
	void setTemporaryGoalPosition(const BWAPI::TilePosition& temporaryPosition);

	/**
	 * Returns the current temporary position if one exists.
	 * @return current temporary position, if no one exists it returns
	 * BWAPI::TilePositions::Invalid.
	 */
	const BWAPI::TilePosition& getTemporaryGoalPosition() const;

	/**
	 * Returns whether the squad has a temporary position or not
	 * @return true if the squad has a temporary goal position.
	 */ 
	bool hasTemporaryGoalPosition() const;

private:
	/**
	 * Handles the regrouping. It will check whether the squad shall regroup (if it's not
	 * regrouping) or continue with the last active goal once the regrouping is finished.
	 */
	void handleRegroup();

	/**
	 * Checks if a unit is out of the config::squad::REGROUP_DISTANCE_BEGIN range.
	 * @return true if squad needs regrouping
	 */
	bool needsRegrouping() const;

	/**
	 * Checks if all units are within config::squad::REGROUP_DISTANCE_BEGIN range.
	 * @return true if squad finished regrouping.
	 */
	bool finishedRegrouping() const;

	/**
	 * Sets a regrouping position and makes the units move to that point instead
	 * @param regroupPosition the position to regroup to, usually this would be
	 * getCenter().
	 */
	void setRegroupPosition(const BWAPI::TilePosition& regorupPosition);

	/**
	 * Clears the regrouping position and resumes moving to the last goal we had.
	 * It will first try the temporary position, then via position, and last the goal
	 * position if none of the previous exists.
	 */
	void clearRegroupPosition();

	/**
	 * Returns true if a unit is standing still and not attacking.
	 * @return true if a unit stands still and not attacks 
	 */
	bool isAUnitStill() const;

	std::vector<std::tr1::shared_ptr<WaitGoal>> mWaitGoals;
	std::vector<UnitAgent*> mUnits;
	std::list<BWAPI::TilePosition> mGoalPositions;
	UnitComposition mUnitComposition;
	BWAPI::TilePosition mTempGoalPosition;
	BWAPI::TilePosition mRegroupPosition;

	bool mDisbandable; /**< If the squad is allowed to be destroyed */
	bool mDisbanded;
	bool mTravelsByAir;
	bool mAvoidEnemyUnits; /**< If the squad shall avoid enemy units at all costs */
	States mState;
	SquadId mId;
	std::tr1::weak_ptr<Squad> mThis;
	GoalStates mGoalState;

	// Calculating furthest unit away distance
	mutable double mFurthestUnitAwayDistance;
	mutable double mFurthestUnitAwayLastTime;

	static int mcsInstance; /**< Number of instances, used for init and release of KeyHandler. */
	static utilities::KeyHandler<_SquadType>* mpsKeyHandler;
	static SquadManager* mpsSquadManager;
};
}