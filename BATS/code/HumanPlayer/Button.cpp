#include "Button.h"
#include <BWAPI/Position.h>
#include <BWAPI/Game.h>

using namespace BWAPI;
using namespace human;

const int TEXT_HEIGHT_HALF = 10;

Button::Button(
	const BWAPI::Position& topLeftPos,
	const BWAPI::Position& bottomRightPos,
	const std::string& buttonText,
	int buttonTextIndent,
	BWAPI::Key hotkey,
	const std::string& sendText)
	:
	mTopLeftPos(topLeftPos),
	mBottomRightPos(bottomRightPos),
	mButtonText(buttonText),
	mButtonTextIndent(buttonTextIndent),
	mHotkey(hotkey),
	mSendText(sendText),
	mPressed(false)
{
	// Does nothing
}

Button::~Button() {
	// Does nothing
}

void Button::update() {
	// Check current state of the mouse
	const Position& mousePos = Broodwar->getMousePosition();
	bool hover = false;

	bool pressedThisTurn = false;

	if (isPositionWithinButton(mousePos)) {
		if (Broodwar->getMouseState(M_LEFT)) {
			// If it wasn't pressed last time, send the message now
			if (!mPressed) {
				Broodwar->sendTextEx(true, "%s", mSendText.c_str());
			}

			pressedThisTurn = true;
		}

		hover = true;
	}

	// Test keyboard instead
	if (!pressedThisTurn) {
		// hotkeys only work if Ctrl & Shift is down
		if (Broodwar->getKeyState(K_CONTROL) && Broodwar->getKeyState(K_SHIFT) && Broodwar->getKeyState(mHotkey)) {
			if (!mPressed) {
				Broodwar->sendTextEx(true, "%s", mSendText.c_str());
			}
			pressedThisTurn = true;
		}
	}

	mPressed = pressedThisTurn;

	// Draw the button
	// Border
	Broodwar->drawBoxScreen(mTopLeftPos.x(), mTopLeftPos.y(),
		mBottomRightPos.x(), mBottomRightPos.y(),
		Colors::Red
	);

	// Inside box (black if not pressed, brighter if hover, grey if pressed)
	Color insideColor = Colors::Black;
	if (mPressed) {
		insideColor = Color(80,80,80);
	} else if (hover) {
		insideColor = Color(40,40,40);
	}

	Broodwar->drawBoxScreen(mTopLeftPos.x()+1, mTopLeftPos.y()+1,
		mBottomRightPos.x()-1, mBottomRightPos.y()-1,
		insideColor, true
	);

	// Button text
	Broodwar->setTextSize(2);
	const Position& textPos = getButtonTextPosition();
	Broodwar->drawTextScreen(textPos.x(), textPos.y(), "\x08%s", mButtonText.c_str());
}

bool Button::isPositionWithinButton(const BWAPI::Position& position) const {
	return position.x() >= mTopLeftPos.x() && position.x() <= mBottomRightPos.x() &&
		position.y() >= mTopLeftPos.y() && position.y() <= mBottomRightPos.y();
}

BWAPI::Position Button::getButtonTextPosition() const {
	int y = (mBottomRightPos.y() - mTopLeftPos.y()) / 2;
	y += mTopLeftPos.y() - TEXT_HEIGHT_HALF;

	int x = mTopLeftPos.x() + mButtonTextIndent;

	return Position(x, y);
}