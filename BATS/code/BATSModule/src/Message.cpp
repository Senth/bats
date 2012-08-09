#include "Message.h"

using namespace bats;

Message::Message() {
	mLastId = static_cast<size_t>(-1);
}

Message::~Message() {
	// Does nothing
}

void Message::addMessage(const std::string& message) {
	mMessages.push_back(message);
}

std::string Message::getMessage() {
	// Skip if no messages
	if (mMessages.empty()) {
		return "";
	}

	// Randomize message when more than one message
	if (mMessages.size() > 1) {
		size_t messageId;
		do {
			messageId = rand() % mMessages.size();
		} while (messageId == mLastId);

		mLastId = messageId;
		return mMessages[messageId];
	}
	// Don't randomize if we only have one message
	else {
		mLastId = 0;
		return mMessages[0];
	}
}

void Message::setName(const std::string& name) {
	mName = name;
}

const std::string& Message::getName() const {
	return mName;
}