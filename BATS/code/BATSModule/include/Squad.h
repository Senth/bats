#pragma once

#include <vector>
#include <memory.h>
#include <BWAPI/TilePosition.h>
#include "SquadDefs.h"
#include "Utilities/KeyType.h"
#include "Utilities/KeyHandler.h"
#include "UnitSet.h"
#include "UnitComposition.h"
#include "UnitCompositionFactory.h"
#include "WaitGoal.h"
#include "Config.h"

// Forward declarations
class UnitAgent;

// Namespace for the project
namespace bats {

class SquadManager;
class GameTime;
class IntentionWriter;

/**
 * Base class for all BATS squads. Represents a squad of units with a shared goal.
 * The squad is composed of different unit types. The squad is by default activated, but
 * can be deactivated to add via paths etc (from the player).
 * @todo improve unit movement when going to a certain location. Make units move to make way
 * for units in the back so the position can be reached.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class Squad : public config::OnConstantChangedListener
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
	 * invalid unit compososition (UnitComposition::INVALID).
	 */
	Squad(const std::vector<UnitAgent*>& units,
		bool avoidEnemyUnits = false,
		bool disbandable = true,
		const UnitComposition& unitComposition = UnitCompositionFactory::INVALID);

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
	 * Returns true if the squad shall avoid enemies
	 * @return true if the squad shall avoid enemies
	 */
	bool isAvoidingEnemies() const;

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
	 * Computes general actions every frame. Calls updateDerived() where 
	 */
	void update();

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
	 * @param unit the unit to add.
	 * @note if the squad has a unit composition, the unit will only be added if the unit
	 * composition has a free spot for it.
	 */
	void addUnit(UnitAgent* unit);

	/**
	 * Adds many units to the squad.
	 * @param units vector with units to add
	 * @note if the squad has a unit composition, units will only be added if the unit
	 * composition has a free spot for them, all other units will be skipped.
	 */
	void addUnits(const std::vector<UnitAgent*>& units);

	/**
	 * Removes a unit from the squad. This should be called when a unit dies too.
	 * @param unit the unit to remove.
	 */
	void removeUnit(UnitAgent* unit);

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
	const BWAPI::TilePosition& getGoalPosition() const;

	/**
	 * Returns all the squad's units
	 * @return all squad's units
	 */
	const std::vector<UnitAgent*>& getUnits();

	/**
	 * Returns all the squad's units
	 * @return all squad's units
	 */
	const std::vector<const UnitAgent*>& getUnits() const;

	/**
	 * Returns this pointer to the squad as an shared_ptr instead of just this
	 * @return shared_ptr to this squad
	 */
	SquadCstPtr getThis() const;

	/**
	 * \copydoc getThis() const
	 */
	SquadPtr getThis();

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
	 * Returns the unit composition of the squad
	 * @return unit composition of the squad, UnitComposition::Invalid
	 */
	const UnitComposition& getUnitComposition() const;

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
	bool isCloseTo(const BWAPI::TilePosition& position, int range) const;

	/**
	 * Returns the name of the squad. Almost purely for debugging purposes
	 * @return name of the derived squad.
	 */
	virtual std::string getName() const = 0;

	/**
	 * Checks whether this squad has air units
	 * @return true if the squad has air units
	 */
	bool hasAir() const;

	/**
	 * Checks whether this squad has ground units
	 * @return true if the squad has ground units
	 */
	bool hasGround() const;

	/**
	 * Checks if this squad has mechanical units
	 * @return true if the squad has mechanical units in it
	 */
	bool hasMechanicalUnits() const;

	/**
	 * Checks if this squad has biological units, units that can be healed by medics
	 * @return true if the squad has biological units
	 */
	bool hasOrganicUnits() const;

	/**
	 * Returns the number of mechanical units in the squad
	 * @return number of mechanical units in the squad
	 */
	size_t getMechanicalUnitCount() const;

	/**
	 * Returns the number of organic units in the squad
	 * @return number of organic units in the squad
	 */
	size_t getOrganicUnitCount() const;

	/**
	 * Checks whether the squad is currently retreating or not. This is not the same as disband
	 * @return true if the squad is retreating.
	 */
	bool isRetreating() const;

	/**
	 * Prints graphical debug information
	 */
	virtual void printGraphicDebugInfo() const;

	/**
	 * Returns the number of units in the squad
	 * @return number of units in the squad
	 */
	size_t getUnitCount() const;

	/**
	 * Returns the supply count of all units in the squad
	 * @note this is actually the double amount of supplies, since that's BWAPI standard
	 * (because Zerglings take up 0.5 supplies)
	 * @return how many supplies the squad occupies.
	 * @see getDeltaSupplyCount()
	 */
	int getSupplyCount() const;

	/**
	 * Returns the delta supply count over the measure_size * measure_interval_time period.
	 * @note this is the double amount of supply since BWAPI uses this supply count
	 * because Zerglings take up 0.5 supply.
	 * @return delta supply count (doubled), if not enough measures it will return 0.
	 * @see getSupplyCount()
	 */
	int getDeltaSupplyCount() const;

	/**
	 * Returns the number of workers in the squad
	 * @return number of workers in the squad
	 */
	size_t getWorkerCount() const;

	virtual void onConstantChanged(config::ConstantName constantName);
	
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
	 * Sets the position to move to as a goal. This will reset all current goal positions.
	 * @param position the new goal position to move to.
	 * @see setViaPath() to set a list of positions, and reset current via positions.
	 * @see addViaPath(BWAPI::TilePosition) to add one position to the back of the queue,
	 * without resetting.
	 * @see addViaPath(std::list<BWAPI::TilePosition>) to add several position to the back
	 * of the queue, without resetting.
	 */
	void setGoalPosition(const BWAPI::TilePosition& position);

	/**
	 * Sets a new via path to the current goal. The bot will always go via the 'via' positions.
	 * @param positions the new via path that will be used. An empty list will remove the current
	 * via path and set them to 0.
	 * @see setGoalPosition() to set one goal position, and reset current positions.
	 * @see addViaPath(BWAPI::TilePosition) to add one via position to the back of the queue.
	 * @see addViaPath(std::list<BWAPI::TilePosition>) to add several position to the back
	 * of the queue.
	 */
	void setViaPath(const std::list<BWAPI::TilePosition>& positions);

	/**
	 * Adds the position to the via path at the back of the queue
	 * @param position the goal position to add at the back of the queue.
	 * @see setGoalPosition() to set one position, and reset current positions.
	 * @see setViaPath() to set a new via path removing the old.
	 * @see addViaPath(std::list<BWAPI::TilePosition>) to add several position to the back
	 * of the queue.
	 */
	void addViaPath(const BWAPI::TilePosition& position);

	/**
	 * Adds the positions at the back of the queue.
	 * @param positions the goal positions to add at the back of the queue.
	 * @see setGoalPosition() to set one position, and reset current positions.
	 * @see setViaPath() to set a new via path removing the old.
	 * @see addViaPath(BWAPI::TilePosition) to add one via position to the back of the queue.
	 */
	void addViaPath(const std::list<BWAPI::TilePosition>& positions);

	/**
	 * Virtual compute actions function. This function is called by update().
	 * Implement this function to create specific squad behavior.
	 */
	virtual void updateDerived();

	/**
	 * Check whether the goal is completed or not. Shall return whether
	 * the current goal state of the squad.
	 * @deprecated Will be removed in future versions. Test this in updateDerived()
	 * instead
	 * @return current goal state of the squad.
	 */
	__declspec(deprecated) virtual GoalStates checkGoalState() const {return GoalState_NotCompleted;}

	/**
	 * Called when a new goal shall be created.
	 * @deprecated Will be removed in future versions. Create the goal in
	 * updateDerived(), constructor, or somewhere else.
	 * @returns true a goal was successfully created.
	 */
	__declspec(deprecated) virtual bool createGoal() {return false;}

	/**
	 * Called when a goal fails. Disbands the squad, derive this function if you want to
	 * create a new repeating goal.
	 * @deprecated Will be removed in future versions.
	 */
	__declspec(deprecated) virtual void onGoalFailed();

	/**
	 * Called when a goal succeeds. Disbands the squad, derive this function if you want to
	 * create a new goal repeating goal.
	 * @deprecated Will be removed in future versions.
	 */
	__declspec(deprecated) virtual void onGoalSucceeded();

	/**
	 * Sets the current retreating goal, this will also disable any regrouping functionality
	 * and temporary goals until the squad has arrived at the retreat position. Via path
	 * can still be used. While retreating it will never check the state of the current
	 * goal either.
	 * @param retreatPosition where the squad shall retreat to
	 */
	void setRetreatPosition(const BWAPI::TilePosition& retreatPosition);

	/**
	 * Returns the retreat position of the squad.
	 * @return the squad's retreat position. TilePositions::Invalid if it doesn't have
	 * any retreat position.
	 */
	const BWAPI::TilePosition& getRetreatPosition() const;

	/**
	 * Called when the squad has arrived at the retreat position, use this in derived
	 * classes if those use the retreat functionality. Default behavior disbands the squad
	 */
	virtual void onRetreatCompleted();

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

	/**
	 * Enable or disable the squad's ability to regroup. By default it is set to true
	 * but for transportations this can be counter-productive while the transports are loading.
	 * @param canRegroup true if the squad shall be able to regroup automatically, false otherwise
	 */
	void setCanRegroup(bool canRegroup);

	/**
	 * Checks whether there are enemy attacking units nearby. Uses the double vision range 
	 * (of the squad unit with longest range) as radius from the center of the squad.
	 * This function will not work if the squad is regrouping (and will then always
	 * return false), because the squad's center might be far away from all squad units.
	 * This takes into account ground structures that can attack as well (canons, sunken colony,
	 * etc.) if the squad has air units it will take into account anti-air structures as well
	 * (turrets, spore colony).
	 * @return true if there are enemies are within sight_distance_multiplier vision range from the center of
	 * the squad. Will always return false if the squad is regrouping.
	 */
	bool isEnemyAttackUnitsWithinSight() const;

	/**
	 * Returns all enemies within sight range from the center of the squad. This can
	 * be a weird behavior if the squad is regrouping and the groups are far away
	 * from each other. Or it could be exactly what we want (if we want to surround
	 * the enemy), but it's generally better to use a customized function then.
	 * Uses sight_distance_multiplier to get the radius out from the center. The radius
	 * is the unit with longest sight range multiplied with the mentioned variable.
	 * @param onlyAttackingUnits set to true if you only want enemy units that can attack
	 * @return enemy units within sight range from the center of the squad.
	 */
	std::vector<BWAPI::Unit*> getEnemyUnitsWithinSight(bool onlyAttackingUnits) const;

	/**
	 * Returns the sight range of this squad. This is the unit in this squad with the
	 * longest sight range multiplied; the multiplier can be set in the config file
	 * under the squad section.
	 * @return unit with longest sight multiplied with sight_distance_multiplier.
	 */
	int getSightDistance() const;

	/**
	 * Clears all movement positions, goal, via, regroup, etc.
	 */
	void clearMovement();

	/**
	 * Sets if the squad shall avoid enemy units
	 * @param bAvoidEnemyUnits if the squad shall avoid enemy units or not
	 */
	void setAvoidEnemyUnits(bool bAvoidEnemyUnits);

	/**
	 * Returns extra graphical debug information about the squad.
	 * @return formatted text for debugging purposes.
	 */
	virtual std::string getDebugString() const;

	/**
	 * Returns true if a unit is standing still and not attacking.
	 * @return true if a unit stands still and not attacks 
	 */
	bool isAUnitStill() const;

#pragma warning(push)
#pragma warning(disable:4100)
	/**
	 * Called when a unit has been successfully added to the squad.
	 * @param pAddedUnit the new unit that has been added
	 */
	virtual void onUnitAdded(UnitAgent* pAddedUnit) {}

	/**
	 * Called when a unit has been successfully removed from the squad.
	 * @param pRemovedUnit the unit that has been removed
	 * @todo reset the unit to its default behavior.
	 */
	virtual void onUnitRemoved(UnitAgent* pRemovedUnit) {}
#pragma warning(pop)

	static GameTime* msGameTime;
	static IntentionWriter* msIntentionWriter;

private:
	/**
	 * Disallow copy constructor. Will generate a compile or link error when used.
	 * @param nonCopyable non copyable object
	 */
	Squad(const Squad& nonCopyable);

	/**
	 * Disallow assignment operator. Will generate a compile or link error when used.
	 * @param nonCopyable non copyable object
	 * @return nonCopyable object
	 */
	Squad& operator=(const Squad& nonCopyable);

	/**
	 * Handle retreats, checks if the retreat is complete and then calls
	 * onRetreatCompleted().
	 */
	void handleRetreat();
	
	/**
	 * Handle goal states, checks whether the goal has succeeded or failed and calls
	 * onGoalSucceeded() or onGoalFailed().
	 */
	void handleGoal();

	/**
	 * Handles wait goals, removes finished wait goals and calls onWaitGoalFinished
	 */
	void handleWaitGoals();

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
	void setRegroupPosition(const BWAPI::TilePosition& regroupPosition);

	/**
	 * Clears the regrouping position and resumes moving to the last goal we had.
	 * It will first try the temporary position, then via position, and last the goal
	 * position if none of the previous exists.
	 */
	void clearRegroupPosition();

	/**
	 * Updates the unit's movement to the current active position to move to.
	 * Call this once a type of goal has been added removed. Or when a unit is added.
	 * @pre squad state is active
	 * @param unit the unit to update the goal position of
	 * @see updateUnitMovement() to update the position of all units in the squad.
	 * @see getPriorityMoveToPosition() for the move to position with the highest priority
	 */
	void updateUnitMovement(UnitAgent* unit);

	/**
	 * Updates all unit movements to the current active position. This function is equivalent
	 * to calling updateUnitMovement(UnitAgent*) for all units in the squad.
	 * @pre squad state is active
	 * @see updateUnitMovement(UnitAgent*) to update the position of one unit.
	 * @see getPriorityMoveToPosition() for the move to position with the highest priority
	 */
	void updateUnitMovement();

	/**
	 * Returns the current "goal" with the highest priority. \n
	 * <b>Priority:</b>
	 * <ol>
	 * <li>RegroupPosition</li>
	 * <li>TemporaryPosition (not used when retreating)</li>
	 * <li>ViaPath</li>
	 * <li>Retreat</li>
	 * <li>GoalPosition</li>
	 * </ol>
	 * @return position with the highest priority to move to.
	 * @see updateUnitMovement(UnitAgent*) to update the position of one unit.
	 * @see updateUnitMovement() to update the position of all units in the squad. 
	 */
	BWAPI::TilePosition getPriorityMoveToPosition() const;

	/**
	 * Updates the supply count
	 */
	void updateSupply();

	/**
	 * Calculates and returns the squad dimension size. Each Large unit is 4 in size,
	 * medium = 2, small = 1.
	 * @return dimension size of squad in small units
	 */
	int getDimensionSize() const;

	/**
	 * Calculates the regroup increment (for begin and end) for this squad
	 * @return the regroup increment to be used for this squad
	 */
	int getRegroupIncrement() const;

	/**
	 * Calculates a new regroup position for the squad and returns it. Currently
	 * takes the unit that is closest to the goal.
	 * @return new regroup position for the squad
	 */
	BWAPI::TilePosition findRegroupPosition() const;

	std::vector<std::tr1::shared_ptr<WaitGoal>> mWaitGoals;
	std::vector<UnitAgent*> mUnits;
	std::list<BWAPI::TilePosition> mViaPath;
	std::list<int> mSupplies;
	BWAPI::TilePosition mGoalPosition;
	BWAPI::TilePosition mTempGoalPosition;
	BWAPI::TilePosition mRegroupPosition;
	BWAPI::TilePosition mRetreatPosition;
	UnitComposition mUnitComposition;
	double mRegroupStartTime;
	bool mCanRegroup;
	bool mDisbandable; /**< If the squad is allowed to be destroyed */
	bool mDisbanded;
	bool mHasAirUnits;
	bool mHasGroundUnits;
	size_t mcMechanicalUnits;
	size_t mcOrganicUnits;
	size_t mcWorkers;
	bool mAvoidEnemyUnits;
	bool mInitialized;
	States mState;
	SquadId mId;
	std::tr1::weak_ptr<Squad> mThis;
	GoalStates mGoalState;
	double mUpdateLast;

	// Calculating furthest unit away distance
	mutable double mFurthestUnitAwayDistance;
	mutable double mFurthestUnitAwayLastTime;

	static int mscInstances; /**< Number of instances, used for init and release of KeyHandler. */
	static utilities::KeyHandler<_SquadType>* msKeyHandler;
	static SquadManager* msSquadManager;
};
}