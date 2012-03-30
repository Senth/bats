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

WaitGoal::WaitGoal(double timeout) {
	mpGameTime = NULL;

	mpGameTime = GameTime::getInstance();
	mWaitState = WaitState_Timeout;
	mTimeout = timeout;
	mUsesTimeout = true;
	mStartTime = mpGameTime->getElapsedTime();
}

WaitGoal::~WaitGoal() {
	// Does nothing
}

void WaitGoal::update() {
	if (mWaitState == WaitState_Waiting && mUsesTimeout) {
		double currentTime = mpGameTime->getElapsedTime();
		double diffTime = currentTime - mStartTime;
		if (diffTime > mTimeout) {
			mWaitState = WaitState_Timeout;
		}
	}
}

WaitStates WaitGoal::getWaitState() const {
	return mWaitState;
}