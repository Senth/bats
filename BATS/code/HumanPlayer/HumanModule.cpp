#include "HumanModule.h"
#include <sstream>
#include <windows.h>

using namespace human;
using namespace BWAPI;

#pragma warning(push)
#pragma warning(disable:4100)
HumanModule::HumanModule() {}
HumanModule::~HumanModule() {}
void HumanModule::onStart() {
	// Sleep, necessary for not crashing BATS bot
#ifdef _DEBUG
	Sleep(2000);
#endif
	
	mDisplayGui = false;

	if (!Broodwar->isFlagEnabled(Flag::UserInput)) {
		Broodwar->enableFlag(Flag::UserInput);
	}

	// Init all buttons
	int offsetXMin = 580;
	int offsetXMax = offsetXMin + 50;
	int ySize = 28;
	int ySpacing = 2;
	int yGroupSpacing = 10;

	int yOffset = 132;
	mButtons.push_back(Button(Position(offsetXMin, yOffset), Position(offsetXMax, yOffset+ySize), "Attack", 4, K_A, "attack")); yOffset += ySize + ySpacing;
	mButtons.push_back(Button(Position(offsetXMin, yOffset), Position(offsetXMax, yOffset+ySize), "Follow", 8, K_F, "follow")); yOffset += ySize + ySpacing;
	mButtons.push_back(Button(Position(offsetXMin, yOffset), Position(offsetXMax, yOffset+ySize), "Drop", 10, K_D, "drop")); yOffset += ySize + ySpacing + yGroupSpacing;

	mButtons.push_back(Button(Position(offsetXMin, yOffset), Position(offsetXMax, yOffset+ySize), "Scout", 6, K_S, "scout")); yOffset += ySize + ySpacing;
	mButtons.push_back(Button(Position(offsetXMin, yOffset), Position(offsetXMax, yOffset+ySize), "Expand", 4, K_E, "expand")); yOffset += ySize + ySpacing;
	mButtons.push_back(Button(Position(offsetXMin, yOffset), Position(offsetXMax, yOffset+ySize), "Trans.", 4, K_T, "transition")); yOffset += ySize + ySpacing;
}
void HumanModule::onEnd(bool isWinner) {}
void HumanModule::onFrame() {
	// draw GUI
	if (mDisplayGui) {
		for (size_t i = 0; i < mButtons.size(); ++i) {
			mButtons[i].update();
		}
	}
}
void HumanModule::onSendText(std::string text) {
	if (startsWith(text, "speed")) {
		int speed = -1;

		std::string speedValueStr = text.substr(5);
		std::stringstream ss;
		ss << speedValueStr;
		ss >> speed;

		Broodwar->setLocalSpeed(speed);
		Broodwar->printf("Global speed set to: %i", speed);
		Broodwar->sendText(text.c_str());
	} else {
		Broodwar->sendText(text.c_str());
	}
}
void HumanModule::onReceiveText(BWAPI::Player* player, std::string text) {
	if (startsWith(text, "speed")) {
		int speed = -1;

		std::string speedValueStr = text.substr(5);
		std::stringstream ss;
		ss << speedValueStr;
		ss >> speed;

		Broodwar->setLocalSpeed(speed);
		Broodwar->printf("Speed set to: %i", speed);
	}
	else if (text == "gui control on") {
		mDisplayGui = true;
	}
	else if (text == "gui control off") {
		mDisplayGui = false;
	}
}
void HumanModule::onPlayerLeft(BWAPI::Player* player) {}
void HumanModule::onNukeDetect(BWAPI::Position target) {}
void HumanModule::onUnitDiscover(BWAPI::Unit* unit) {}
void HumanModule::onUnitEvade(BWAPI::Unit* unit) {}
void HumanModule::onUnitShow(BWAPI::Unit* unit) {}
void HumanModule::onUnitHide(BWAPI::Unit* unit) {}
void HumanModule::onUnitCreate(BWAPI::Unit* unit) {}
void HumanModule::onUnitDestroy(BWAPI::Unit* unit) {}
void HumanModule::onUnitMorph(BWAPI::Unit* unit) {}
void HumanModule::onUnitRenegade(BWAPI::Unit* unit) {}
void HumanModule::onSaveGame(std::string gameName) {}
void HumanModule::onUnitComplete(BWAPI::Unit *unit) {}
#pragma warning(pop)

bool HumanModule::startsWith(const std::string& text,const std::string& token) {

	if(text.length() < token.length() || text.length() == 0 || token.length() == 0)
		return false;

	for(unsigned int i=0; i<token.length(); ++i)
	{
		if(text[i] != token[i])
			return false;
	}

	return true;
}