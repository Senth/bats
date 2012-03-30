#pragma once

#include <memory.h>
#include <map>
#include <string>

// Namespace for the project
namespace bats {

// Forward declarations
class WaitGoal;

/**
 * Handles all the wait goals. I.e. it updates them and removes completed
 * wait goals. Note that other pointers will still be valid since
 * shared_ptr is used.
 * 
 * Wait goals are combined in sets, i.e. the set has a name (e.g. attack_coordinator)
 * and all wait goals belonging to that set can easily be returned.
 * 
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class WaitGoalManager {
public:
	/**
	 * Destructor
	 */
	virtual ~WaitGoalManager();

	/**
	 * Returns the instance of WaitGoalManager.
	 * @return instance of WaitGoalManager.
	 */
	static WaitGoalManager* getInstance();

	/**
	 * Updates all wait goals, and removes those that are finished.
	 */
	void update();

	/**
	 * Add a new WaitGoal
	 * @param waitGoal the new WaitGoal to add.
	 * @param setName optional set name the WaitGoal shall belong to. Defaults to "default".
	 */
	void addWaitGoal(const std::tr1::shared_ptr<WaitGoal>& waitGoal, const std::string& setName = "default");

	/**
	 * Returns all wait goals that belongs to a set.
	 * @param setName name of the set the wait goals belong to
	 */
	std::pair<
		std::multimap<std::string, std::tr1::shared_ptr<WaitGoal>>::const_iterator,
		std::multimap<std::string, std::tr1::shared_ptr<WaitGoal>>::const_iterator
	>
		getWaitGoalsBySet(const std::string& setName) const;

private:
	/**
	 * Singleton constructor to enforce singleton usage.
	 */
	WaitGoalManager();

	std::multimap<std::string, std::tr1::shared_ptr<WaitGoal>> mWaitGoals;

	static WaitGoalManager* mpsInstance;
};
}