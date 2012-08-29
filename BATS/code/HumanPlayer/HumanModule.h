#pragma once

#include "Button.h"
#include <BWAPI.h>
#include <string>
#include <vector>

// Namespace for the project
namespace human {

/**
 * Module for the human player. Simple acts as a wrapper to be able to play against
 * the other AI on the same computer.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class HumanModule : public BWAPI::AIModule {
public:
	/**
	 * Default constructor
	 */
	HumanModule();

	/**
	 * Destructor
	 */
	virtual ~HumanModule();


	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player* player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player* player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit* unit);
	virtual void onUnitEvade(BWAPI::Unit* unit);
	virtual void onUnitShow(BWAPI::Unit* unit);
	virtual void onUnitHide(BWAPI::Unit* unit);
	virtual void onUnitCreate(BWAPI::Unit* unit);
	virtual void onUnitDestroy(BWAPI::Unit* unit);
	virtual void onUnitMorph(BWAPI::Unit* unit);
	virtual void onUnitRenegade(BWAPI::Unit* unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit *unit);
private:

	bool startsWith(const std::string& text,const std::string& token);

	std::vector<Button> mButtons;
};
}