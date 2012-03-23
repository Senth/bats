#include "GameTime.h"
#include <BWAPI/Game.h>

using namespace bats;

const float SECONDS_PER_FRAME_FASTEST = 0.042f;

GameTime* GameTime::mpsInstance = NULL;

GameTime::GameTime() {
	mStartFrame = BWAPI::Broodwar->getFrameCount();
	mElapsedTime = 0.0f;
}

GameTime::~GameTime()  {
	// Does nothing
}

GameTime* GameTime::getInstance() {
	if (mpsInstance == NULL) {
		mpsInstance = new GameTime();
	}
	return mpsInstance;
}

void GameTime::update() {
	int currentFrame = BWAPI::Broodwar->getFrameCount();
	mElapsedTime = static_cast<float>(currentFrame - mStartFrame) * SECONDS_PER_FRAME_FASTEST;
}

float GameTime::getElapsedTime() const {
	return mElapsedTime;
}