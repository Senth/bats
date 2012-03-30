#include "WaitGoalManager.h"
#include "WaitGoal.h"
#include <cstdlib> // For NULL

using namespace bats;
using namespace std;
using std::tr1::shared_ptr;

WaitGoalManager* WaitGoalManager::mpsInstance = NULL;

WaitGoalManager::WaitGoalManager() {
	// Does nothing
}

WaitGoalManager::~WaitGoalManager() {
	mpsInstance = NULL;
}

WaitGoalManager* WaitGoalManager::getInstance() {
	if (NULL == mpsInstance) {
		mpsInstance = new WaitGoalManager();
	}
	return mpsInstance;
}

void WaitGoalManager::update() {
	multimap<string, shared_ptr<WaitGoal>>::iterator goalIt = mWaitGoals.begin();
	while (goalIt != mWaitGoals.end()) {
		shared_ptr<WaitGoal> currentGoal = (goalIt->second);

		currentGoal->update();

		// Remove completed goals
		if (currentGoal->getWaitState() != WaitState_Waiting) {
			goalIt = mWaitGoals.erase(goalIt);
		} else {
			++goalIt;
		}
	}
}

void WaitGoalManager::addWaitGoal(const shared_ptr<WaitGoal>& waitGoal, const string& setName) {
	mWaitGoals.insert(make_pair(setName, waitGoal));
}

pair<
	multimap<string, shared_ptr<WaitGoal>>::const_iterator,
	multimap<string, shared_ptr<WaitGoal>>::const_iterator
>
	WaitGoalManager::getWaitGoalsBySet(const string& setName) const
{
	return mWaitGoals.equal_range(setName);
}