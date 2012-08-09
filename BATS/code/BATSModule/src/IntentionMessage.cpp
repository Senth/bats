#include "IntentionMessage.h"
#include "GameTime.h"

using namespace bats;

GameTime* IntentionMessage::msGameTime = NULL;
double IntentionMessage::msDefaultIntervalTimeMin = 0.0;

IntentionMessage::IntentionMessage() {
	mLastCallTime = 0.0;
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
	if (msGameTime->getElapsedTime(mLastCallTime) < mIntervalTimeMin) {
		return "";
	}
	mLastCallTime = msGameTime->getElapsedTime();

	return Message::getMessage();
}