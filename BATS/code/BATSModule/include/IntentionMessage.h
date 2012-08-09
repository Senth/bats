#pragma once

#include "Message.h"

// Namespace for the project
namespace bats {

// Forward declarations
class GameTime;

/**
 * Hold messages for the specific intention.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class IntentionMessage : public Message {
public:
	/**
	 * Default constructor. Does not contain any messages.
	 */
	IntentionMessage();

	/**
	 * Sets the minimum interval time between messages
	 * @param intervalMin minimum interval time between messages
	 * @see setDefaultIntervalTimeMin()
	 */
	void setIntervalTimeMin(double intervalMin);

	/**
	 * Sets the default minimum interval time for all intent messages
	 * @param intervalMin minimum interval time between messages
	 * @see setIntervalTimeMin()
	 */
	static void setDefaultIntervalTimeMin(double intervalMin);

	/**
	 * Destructor
	 */
	virtual ~IntentionMessage();

	/**
	 * Returns a random message, but not the last, for this intention. If this is
	 * called too soon before the minimum interval time has passed an empty string
	 * will be returned.
	 * @return random message, empty string if called too soon.
	 */
	virtual std::string getMessage();

private:
	double mLastCallTime;
	double mIntervalTimeMin;
	static double msDefaultIntervalTimeMin;
	static GameTime* msGameTime;
};
}