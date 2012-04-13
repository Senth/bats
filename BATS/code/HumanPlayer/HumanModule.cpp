#include "HumanModule.h"

using namespace human;
using namespace BWAPI;

#pragma warning(push)
#pragma warning(disable:4100)
HumanModule::HumanModule() {}
HumanModule::~HumanModule() {}
void HumanModule::onStart() {}
void HumanModule::onEnd(bool isWinner) {}
void HumanModule::onFrame() {
	if (!Broodwar->isFlagEnabled(Flag::UserInput)) {
		Broodwar->enableFlag(Flag::UserInput);
		Broodwar->printf("Hello");
	}
}
void HumanModule::onSendText(std::string text) {
	if (text == "test") {
		Broodwar->printf("It's working!");
	}
}
void HumanModule::onReceiveText(BWAPI::Player* player, std::string text) {}
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