#include "WaitGoal.h"
#include "GameTime.h"
#include <cstdlib>

using namespace bats;

WaitGoal::WaitGoal() {
	mpGameTime = NULL;

	mpGameTime = GameTime::getInstance();
	mWaitState = WaitState_Timeout;
	mTimeout = 0.0f;
	mStartTime = 0.0f;
	mUsesTimeout = false;
}

WaitGoal::WaitGoal(int timeout) {
	mpGameTime = NULL;

	mpGameTime = GameTime::getInstance();
	mWaitState = WaitState_Timeout;
	mTimeout = static_cast<float>(timeout);
	mUsesTimeout = true;
	mStartTime = mpGameTime->getElapsedTime();
}

WaitGoal::~WaitGoal() {
	// Does nothing
}

void WaitGoal::computeActions() {
	if (mUsesTimeout) {
		float currentTime = mpGameTime->getElapsedTime();
		float diffTime = currentTime - mStartTime;
		if (diffTime > mTimeout) {
			mWaitState = WaitState_Timeout;
		}
	}
}

WaitStates WaitGoal::getWaitState() const {
	return mWaitState;
}