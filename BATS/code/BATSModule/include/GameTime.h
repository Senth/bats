#pragma once


// Namespace for the project
namespace bats {

/**
 * Keeps track of the game time in seconds
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class GameTime {
public:
	/**
	 * Destructor
	 */
	virtual ~GameTime();

	/**
	 * Returns the instance of GameTimer.
	 */
	static GameTime* getInstance();

	/**
	 * Updates the game time to the current specific time. Use getTime() to actually get
	 * the number of seconds passed since the game started.
	 */
	void update();

	/**
	 * Returns the number of seconds elapsed since the game started. One second corresponds
	 * to one game second with the fastest game speed in StarCraft. Note fastest as in specifying
	 * speed with /speed -1 and then setting the fastest game speed in the options.
	 * @return elapsed game seconds since start of game.
	 */
	double getElapsedTime() const;

	/**
	 * Returns the number of seconds since the specified frame. One second corresponds to one
	 * game second with the fastest game speed.
	 * @param sinceFrame from what frame we want to calculate the elapsed seconds
	 * @return elapsed game seconds since the specified frame
	 */
	double getElapsedTime(int sinceFrame) const;
	
private:
	/**
	 * Singleton constructor to enforce singleton usage.
	 */
	GameTime();

	int mStartFrame;
	double mElapsedTime;

	static GameTime* mpsInstance;
};
}