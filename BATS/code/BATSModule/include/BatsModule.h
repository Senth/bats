#pragma once

#include "BTHAIModule/Source/BTHAIModule.h"

// Forward declarations
class Profiler;
class AgentManager;

// Namespace for the project
namespace bats {

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
	 * Called when a player leaves the game
	 * @param player the player that left the game
	 */
	virtual void onPlayerLeft(BWAPI::Player* player);

	/**
	 * Called when a unit has been created
	 * @param pUnit the newly created unit
	 */
	virtual void onUnitCreate(BWAPI::Unit* pUnit);

	/**
	 * Called when a unit has been destroyed
	 * @param pUnit the destroyed unit
	 */
	virtual void onUnitDestroy(BWAPI::Unit* pUnit);

	/**
	 * Called when a unit has morphed
	 * @param pUnit the morphed unit
	 */
	virtual void onMorphUnit(BWAPI::Unit* pUnit);

protected:
	/**
	 * Checks if we have lost the game. This includes low workers, no army, no money to build
	 * workers.
	 * @return true if we have lost the game.
	 */
	virtual bool isGameLost() const;

	/**
	 * Checks whether we are actually playing a game, i.e. not a reply and the game has started.
	 * Note that it will return true even if the game is paused.
	 * @return true if the game is being played
	 */
	virtual bool areWePlaying() const;

	Profiler* mpProfiler;
	AgentManager* mpAgentManager;

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
	* custom function for finding if the string starts with the given token
	*/
	bool startsWith(const std::string& text,const std::string& token);

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

	int mDebugLevel;
};
}