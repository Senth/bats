#pragma once

#include <string>
#include <BWAPI/Position.h>
#include <BWAPI/Input.h>

// Namespace for the project
namespace human {

/**
 * An GUI button that sends a text message when pressed.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class Button {
public:
	/**
	 * Default constructor
	 * @param topLeftPos the top left position on the screen
	 * @param bottomRightPos the bottom right position on the screen
	 * @param buttonText text that will appear on the button
	 * @param buttonTextIndent how many pixels the button text shall be indented
	 * @param hotkey the hotkey that can activate the button too
	 * @param sendText message to send when the button is presses
	 */
	Button(
		const BWAPI::Position& topLeftPos,
		const BWAPI::Position& bottomRightPos,
		const std::string& buttonText,
		int buttonTextIndent,
		BWAPI::Key hotkey,
		const std::string& sendText
	);

	/**
	 * Destructor
	 */
	virtual ~Button();

	/**
	 * Updates the state of the button and checks if it's pressed or not.
	 * If it is, it will send the message.
	 * Also draws the actual button.
	 */
	void update();

private:
	/**
	 * Checks if the positon is within the button's area
	 * @param position the position to check
	 * @return true if the position is within the button's area
	 */
	bool isPositionWithinButton(const BWAPI::Position& position) const;

	/**
	 * Calculates and returns the position where the text shall begin to be displayed
	 * @return position of text print-out.
	 */
	BWAPI::Position getButtonTextPosition() const;

	bool mPressed;
	BWAPI::Key mHotkey;
	int mButtonTextIndent;
	std::string mButtonText;
	std::string mSendText;
	BWAPI::Position mTopLeftPos;
	BWAPI::Position mBottomRightPos;
};
}