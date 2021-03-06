#include "GameTime.h"
#include <BWAPI/Game.h>

using namespace bats;

const double SECONDS_PER_FRAME_FASTEST = 0.042;
const double FRAMES_PER_SECOND = 1.0 / SECONDS_PER_FRAME_FASTEST;

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
	mElapsedTime = static_cast<double>(currentFrame - mStartFrame) * SECONDS_PER_FRAME_FASTEST;
}

double GameTime::getElapsedTime(double sinceSecond) const {
	return mElapsedTime - sinceSecond;
}

double GameTime::getElapsedTime(int sinceFrame) const {
	int frameDiff = BWAPI::Broodwar->getFrameCount() - sinceFrame;
	return static_cast<double>(frameDiff) * SECONDS_PER_FRAME_FASTEST;
}

int GameTime::getFrameCount(int sinceFrame) const {
	return BWAPI::Broodwar->getFrameCount() - sinceFrame;
}

double GameTime::convertFramesToSeconds(int frames) {
	return frames * SECONDS_PER_FRAME_FASTEST;
}

int GameTime::convertSecondsToFrames(double seconds) {
	return static_cast<int>(seconds * FRAMES_PER_SECOND);
}