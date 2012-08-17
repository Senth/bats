#pragma once

#include "BTHAIModule/Source/BTHAIModule.h"

// Forward declarations
class Profiler;

// Namespace for the project
namespace bats {

class UnitManager;
class Commander;
class ResourceCounter;
class ExplorationManager;
class WaitGoalManager;
class SquadManager;
class GameTime;
class PlayerArmyManager;
class DefenseManager;
class IntentionWriter;

/**
 * Main module for BATS
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class BatsModule : public BTHAIModule
{
public:
	/**
	 * Default constructor
	 */
	BatsModule();

	/**
	 * Destructor
	 */
	virtual ~BatsModule();

	/**
	 * Called when the game starts, overridden from BTHAI
	 */
	virtual void onStart();

	/**
	 * Called when the game stops, overridden from BTHAI
	 */
	virtual void onEnd();

	/**
	 * Called each frame
	 */
	virtual void onFrame();

	/**
	 * Called when we want to send text
	 * @param text the text that we're sending
	 */
	virtual void onSendText(std::string text);

	/**
	 * Called when received text from another player
	 * @param pPlayer the player that sent the text
	 * @param text the text that we received
	 */
	virtual void onReceiveText(BWAPI::Player* pPlayer, std::string text);

	/**
	 * Called when a player leaves the game
	 * @param pPlayer the player that left the game
	 */
	virtual void onPlayerLeft(BWAPI::Player* pPlayer);

	/**
	 * Called when a nuke is launched by a player
	 * @param target the position of the nuke. If complete map information is disabled and the
	 * target position is not visible, target will be set to Positions::Unknown
	 */
	virtual void onNukeDetect(BWAPI::Position target);

	/**
	 * Called when a unit becomes accessible. If complete map information is enabled, this
	 * will be called at the same time as onUnitCreate(), otherwise it will be called at the
	 * same time as onUnitShow()
	 * @param unit the unit that was discovered
	 */
	virtual void onUnitDiscover(BWAPI::Unit* unit);

	/**
	 * Called right before a unit becomes inaccessible. If complete map information is enabled,
	 * this will be called at the same time as onUnitDestroy(), otherwise it will be called at the
	 * same time as onUnitHide()
	 * @param unit the unit that evaded
	 */
	virtual void onUnitEvade(BWAPI::Unit* unit);

	/**
	 * Called when a unit becomes visible. If complete map information is disabled, this also
	 * means that the unit has just become accessible (onUnitDiscover())
	 * @param unit the unit that has now become visible, includes our units
	 */
	virtual void onUnitShow(BWAPI::Unit* unit);

	/**
	 * Called right before a unit becomes invisible. If complete map information is disabled,
	 * this also means that the unit is about to become inaccessible (onUnitEvade())
	 * @param unit the unit that just was hidden
	 */
	virtual void onUnitHide(BWAPI::Unit* unit);

	/**
	 * Called when an accessible unit is created. If the unit is not accessible at the time of
	 * creation (i.e. if the unit is invisible and complete information is disabled), then this
	 * callback will NOT be called. If the unit is visible at the time of creation, onUnitShow()
	 * will also be called.
	 * @note This is NOT called when a unit changes type (such as larva into egg, or egg into
	 * drone). Building a Refinery/Assimilator/Extractor will not produce an onUnitCreate call
	 * because the Vespene Geyser changes to the unit type of Refinery/Assimilator/Extractor
	 * @param unit the newly created unit.
	 */
	virtual void onUnitCreate(BWAPI::Unit* unit);

	/**
	 * Called when a unit dies or otherwise removed from the game (i.e. a mined out mineral patch).
	 * If the unit is not accessible at the time of destruction, (i.e. if the unit is invisible and
	 * complete map information is disabled), then this callback will NOT be called. If the unit was
	 * visible at the time of destruction, onUnitHide() will also be called.
	 * @note When a Zerg drone becomes an extractor, the Vespene geyser changes to the Zerg
	 * Extractor type and the drone is destroyed.
	 * @param unit the destroyed unit
	 */
	virtual void onUnitDestroy(BWAPI::Unit* unit);

	/**
	 * Called when an accessible unit changes type, such as from a Zerg Drone to a Zerg Hatchery,
	 * or from a Terran Siege Tank Tank Mode to Terran Siege Tank Siege Mode. This is not called
	 * when the type changes to or from UnitTypes::Unknown (which happens when a unit is
	 * transitioning to or from inaccessibility)
	 * @param unit the morphed unit
	 */
	virtual void onUnitMorph(BWAPI::Unit* unit);

	/**
	 * Called when an accessible unit changes ownership, e.g. using a Dark Archon.
	 * @param unit the unit that changed ownership
	 */
	virtual void onUnitRenegade(BWAPI::Unit* unit);

	/**
	 * Called when the user saves the match. 
	 * @param gameName name the player entered in the save game screen.
	 */
	virtual void onSaveGame(std::string gameName);

protected:
	/**
	 * Checks if we have lost the game. This includes low workers, no army, no money to build
	 * workers.
	 * @return true if we have lost the game.
	 */
	virtual bool isGameLost() const;

	/**
	 * Checks whether we are actually playing a game, i.e. not a replay and the game has started.
	 * Note that it will return true even if the game is paused.
	 * @return true if the game is being played
	 */
	virtual bool areWePlaying() const;

	Profiler* mProfiler;
	UnitManager* mUnitManager;
	Commander* mCommander;
	ResourceCounter* mResourceCounter;
	ExplorationManager* mExplorationManager;
	WaitGoalManager* mWaitGoalManager;
	SquadManager* mSquadManager;
	GameTime* mGameTime;
	PlayerArmyManager* mPlayerArmyManager;
	DefenseManager* mDefenseManager;
	IntentionWriter* mIntentionWriter;

private:
	/**
	 * This function shall be called every frame when a game is active.
	 * It updates all game classes. Override this if you want to you own added classes, i.e.
	 * not derived from any of the following classes:
	 * BuildPlanner (BuildOrderManager)
	 * CoverMap
	 * UpgradesPlanner
	 * ResourceManager
	 * PathFinder
	 */
	virtual void updateGame();

	/**
	 * Initializes all the game objects. Override this if you want to use your own classes.
	 */
	virtual void initGameClasses();

	/**
	 * Erase game objects. Override this if you have added extra classes that aren't deleted
	 * by default. Instances or derived instances deleted by default:
	 * BuildPlanner (BuildOrderManager)
	 * CoverMap
	 * UpgradesPlanner
	 * ResourceManager
	 * PathFinder
	 */
	virtual void releaseGameClasses();

	/**
	 * Show debug information to the game window. Credit to Johan Hagelbäck as this function
	 * is taken directly from AILoop::show_debug() except from changing of variable names and layout
	 * to BATS coding standards.
	 */
	virtual void showDebug() const;

	/**
	 * Draw terrain data. Credit to Johan Hagelbäck as this function is taken directly from
	 * AILoop::drawTerrainData() except from changing of variable names and layout to BATS
	 * coding standards.
	 */
	virtual void drawTerrainData() const;
};
}