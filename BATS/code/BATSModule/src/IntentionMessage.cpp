#include "IntentionMessage.h"
#include "GameTime.h"
#include <float.h>

using namespace bats;

GameTime* IntentionMessage::msGameTime = NULL;
double IntentionMessage::msDefaultIntervalTimeMin = 0.0;

IntentionMessage::IntentionMessage() {
	mLastCallTime = -DBL_MAX;
	mIntervalTimeMin = 0.0;

	if (NULL == msGameTime) {
		msGameTime = GameTime::getInstance();
	}
}

IntentionMessage::~IntentionMessage() {
	// Does nothing
}

void IntentionMessage::setIntervalTimeMin(double intervalMin) {
	mIntervalTimeMin = intervalMin;
}

void IntentionMessage::setDefaultIntervalTimeMin(double intervalMin) {
	msDefaultIntervalTimeMin = intervalMin;
}

std::string IntentionMessage::getMessage() {
	// Not enough time passed -> return empty message
	double secondsSinceLastTime = msGameTime->getElapsedTime(mLastCallTime);
	if ((mIntervalTimeMin > 0 && secondsSinceLastTime < mIntervalTimeMin) ||
		(mIntervalTimeMin == 0 && secondsSinceLastTime < msDefaultIntervalTimeMin))
	{
		return "";
	}

	mLastCallTime = msGameTime->getElapsedTime();

	return Message::getMessage();
}