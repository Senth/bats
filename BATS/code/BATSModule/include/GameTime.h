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
	 * Returns the number of seconds since the specified game time. One second corresponds
	 * to one game second with the fastest game speed in StarCraft.
	 * @note Fastest as in specifying speed with /speed -1 and then setting the fastest game speed
	 * in the options.
	 * @param seconds from what game time to calculate the elapsed seconds, defaults to 0.0.
	 * @return elapsed game seconds since the specified game time.
	 */
	double getElapsedTime(double seconds = 0.0) const;

	/**
	 * Returns the number of seconds since the specified frame. One second corresponds to one
	 * game second with the fastest game speed.
	 * @param sinceFrame from what frame we want to calculate the elapsed seconds
	 * @return elapsed game seconds since the specified frame
	 */
	double getElapsedTime(int sinceFrame) const;

	/**
	 * Returns the current frame count. This is equivalent to Broodwar->getFrameCount().
	 * @return number of frames since the start of the game
	 */
	int getFrameCount() const;
	
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