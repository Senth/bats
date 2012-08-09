#pragma once

#include <vector>
#include <string>

// Namespace for the project
namespace bats {

/**
 * Holds messages
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class Message {
public:
	/**
	 * Default constructor
	 */
	Message();

	/**
	 * Destructor
	 */
	virtual ~Message();

	/**
	 * Adds a messages to choose from.
	 * @param message a new message to add
	 */
	void addMessage(const std::string& message);

	/**
	 * Returns a random message, but not the last, for this message.
	 * @return random message, empty string if called too soon.
	 */
	virtual std::string getMessage();

	/**
	 * Sets the name of the message
	 * @param name name of the message
	 */
	void setName(const std::string& name);

	/**
	 * Returns the name of the message
	 * @return name of the message
	 */ 
	const std::string& getName() const;

private:
	std::vector<std::string> mMessages;
	size_t mLastId;
	std::string mName;
};
}