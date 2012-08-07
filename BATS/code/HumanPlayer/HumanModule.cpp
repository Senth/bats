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
	Sleep(1000);
#endif
}
void HumanModule::onEnd(bool isWinner) {}
void HumanModule::onFrame() {
	if (!Broodwar->isFlagEnabled(Flag::UserInput)) {
		Broodwar->enableFlag(Flag::UserInput);
		Broodwar->printf("Hello");
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