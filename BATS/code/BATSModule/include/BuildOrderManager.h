#pragma once

#include "BTHAIModule/Source/BuildPlanner.h"

// Namespace for the project
namespace bats {

/**
 * Manages build orders in three phases: early, mid, and late game.
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 */
class BuildOrderManager : public BuildPlanner
{
public:
	/**
	 * Destructor.
	 */
	virtual ~BuildOrderManager();

	/**
	 * Creates and initializes the class. To get an instance of the class
	 * use BuildPlanner::getInstance(), be sure not to use getInstance() before
	 * initializing this class since it will automatically create a BuildPlanner
	 * instance instead of BuildOrderManager.
	 * @pre BuildPlanner hasn't been initialized, i.e. no call to BuildPlanner::getInstance()
	 */
	void init();
	
private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	BuildOrderManager();

};
}