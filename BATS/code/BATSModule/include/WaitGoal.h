#pragma once


// Namespace for the project
namespace bats {

// Forward declarations
class GameTime;

/**
 * All different wait states
 */
enum WaitStates {
	WaitState_First = 0,
	WaitState_Success = WaitState_First,
	WaitState_Waiting,
	WaitState_Failed,
	WaitState_Timeout,
	WaitState_Lim
};

/**
 * Base class for waiting goals. Use this when you want to wait for a certain action (from
 * derived classes) like WaitReadySquad that waits for when a squad is ready to attack.
 * 
 * You can also use this class itself as a timer. Note the timeout is specified in game seconds
 * when the game is set to fast.
 * 
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class WaitGoal {
public:
	/**
	 * Constructs a WaitGoal without a timeout.
	 */
	WaitGoal();

	/**
	 * Constructs a WaitGoal with the specified timeout. When it has waited that
	 * many seconds it will timeout and returned failed.
	 * @param timeout how long time it shall take for the goal to timeout.
	 */
	WaitGoal(int timeout);

	/**
	 * Destructor.
	 */
	virtual ~WaitGoal();

	/**
	 * Updates the wait goal. Don't forget to call this function from the derived class.
	 */
	virtual void computeActions();

	/**
	 * Returns the state of the goal
	 * @return current state of the wait goal. Can be any of WaitStates enumerations.
	 */
	WaitStates getWaitState() const;

protected:
	WaitStates mWaitState;
	GameTime* mpGameTime;

private:
	float mStartTime;
	float mTimeout;
	bool mUsesTimeout;
};
}