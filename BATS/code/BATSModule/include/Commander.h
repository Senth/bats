#pragma once

#include <vector>
#include <map>
#include <memory.h>
#include <BWAPI/TilePosition.h>
#include "SquadDefs.h"
#include "IntentionWriter.h"

// Namespace for the project
namespace bats {

// Forward declarations
class SquadManager;
class UnitCompositionFactory;
class UnitManager;
class PlayerArmyManager;
class DefenseManager;
class SelfClassifier;
class AlliedClassifier;
class GameTime;
class BuildPlanner;

/**
 * The commander creates squads and sends them out to various locations. The squads are
 * created through commands and can both be created by the teammate player and by the
 * Commander itself. The commander always evaluates the teammate player's commands to ensure
 * that they are valid and can be carried out.
 * 
 * The commander also prints out information messages what this AI is up to, when issuing commands
 * or if they are invalid, in that case why.
 * 
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class Commander {
public:
	/**
	 * Destructor
	 * @pre ScoutManager has not been deleted
	 */
	virtual ~Commander();

	/**
	 * Returns the instance of the Commander
	 * @return instance of the Commander
	 */
	static Commander* getInstance();

	/**
	 * Called every frame. Evaluates the situation if squads needs to be disbanded
	 * merged etc.
	 */
	virtual void computeActions();

	/**
	 * Tries to issue a specified command. This is essentially creating a new squad
	 * or adding a new goal to an existing squad.
	 * @param command the command to try to issue.
	 * @return true if the command will be issued, false otherwise.
	 * @pre the command needs to be an available command, check with isCommandAvailable(), else
	 * nothing will happen and either true or false will be returned.
	 * @note The command will not be issued immediately; although it will assemble the
	 * squad (if it can). It will first wait for a ping for x seconds (specified in)
	 * BATS-data\config.ini, if a ping has been assigned it will then wait for a second ping
	 * for y seconds. If a new command is specified it will complete the current command.
	 */
	virtual void issueCommand(const std::string& command);

	/**
	 * Checks whether the command is an available command that the Commander should handle.
	 * @param command the command to check if it is an command for the Commander.
	 * @return true if the command is available, else false.
	 */
	virtual bool isCommandAvailable(const std::string& command) const;
	
private:
	/**
	 * Private constructor to enforce singleton pattern.
	 */
	Commander();

	/**
	 * All command that can be ordered to the commander, both from allied and the bot itself
	 */
	enum Commands {
		Command_Attack,
		Command_Drop,
		Command_Expand,
		Command_Join,
		Command_Scout,
		Command_Transition,

		Command_Lim
	};

	/**
	 * Tries to issue an command
	 * @param command the command to issue
	 * @param alliedOrdered true if it was the allied player that ordered the command
	 * @param reason optional reason for the command, used when the command is successful
	 * and a reason should be specified why the command was issued. Defaults to Reason_Lim which will
	 * not write any reason.
	 */
	virtual void issueCommand(Commands command, bool alliedOrdered, Reasons reason = Reason_Lim);

	/**
	 * Computes reactive player behavior, i.e. if it shall attack when an allied moves
	 * out to attack etc.
	 */
	void computeAlliedReactions();

	/**
	 * Computes own reactions, or own initiative, such as attacking when expanding
	 * sending out a scout etc.
	 */
	void computeOwnReactions();

	/**
	 * Finishes and executes the squad command ordered by the player.
	 */
	void finishAlliedCreatingCommand();

	/**
	 * Initiates a frontal attack or adds new units if a frontal attack already exist
	 * @param alliedOrdered if it was the allied who ordered the attack
	 * @param reason an optional reason for the command, used when the command is successful
	 * and a reason should be specified why the command was issued. Defaults to Reason_Lim,
	 * i.e. it will not print out any reason.
	 */
	void orderAttack(bool alliedOrdered, Reasons reason = Reason_Lim);

	/**
	 * Either
	 * \li If no frontal attack exists -> Creates a new frontal attack that will follow the
	 * current allied frontal attack;
	 * \li If a frontal attack exist and is following an allied attack -> Adds new units to
	 * the attack; or
	 * \li If a frontal attack exist but is not following an allied attack -> Starts to follow
	 * the allied frontal attack with the current units, i.e. no units are added.
	 * @param alliedOrdered if it was the allied who ordered the drop
	 * @param reason an optional reason for the command, used when the command is successful
	 * and a reason should be specified why the command was issued. Defaults to Reason_Lim,
	 * i.e. it will not print out any reason.
	 */
	void orderJoin(bool alliedOrdered, Reasons reason = Reason_Lim);

	/**
	 * Initiates a drop
	 * @param alliedOrdered if it was the allied who ordered the drop
	 * @param reason an optional reason for the command, used when the command is successful
	 * and a reason should be specified why the command was issued. Defaults to Reason_Lim,
	 * i.e. it will not print out any reason.
	 */
	void orderDrop(bool alliedOrdered, Reasons reason = Reason_Lim);

	/**
	 * Initiates a scout
	 * @param alliedOrdered if it was the allied who ordered the scout
	 * @param reason an optional reason for the command, used when the command is successful
	 * and a reason should be specified why the command was issued. Defaults to Reason_Lim,
	 * i.e. it will not print out any reason.
	 */
	void orderScout(bool alliedOrdered, Reasons reason = Reason_Lim);

	/**
	 * Initiates the expand command
	 * @param alliedOrdered if it was the allied who ordered the scout
	 * @param reason an optional reason for the command, used when the command is successful
	 * and a reason should be specified why the command was issued. Defaults to Reason_Lim,
	 * i.e. it will not print out any reason.
	 */
	void orderExpand(bool alliedOrdered, Reasons reason = Reason_Lim);

	/**
	 * Transitions to the next phase in game, i.e. early -> mid -> late-game.
	 * @param alliedOrdered if it was the allied who ordered the transition
	 * @param reason an optional reason for the command, used when the command is successful
	 * and a reason should be specified why the command was issued. Defaults to Reason_Lim,
	 * i.e. it will not print out any reason.
	 */
	void orderTransition(bool alliedOrdered, Reasons reason = Reason_Lim);

	/**
	 * Initialize string to enumerations for the commands
	 */
	void initStringToEnums();

	/**
	 * Checks if the allied has an active ordered command being created.
	 * @return true if the allied has an active command that is currently being created
	 */
	bool isAlliedCreatingCommand() const;

	/**
	 * Adds repair workers (SCVs) to an existing attack squad
	 * @param attackSquad the attack squad to add repair workers to
	 */
	void addRepairWorkersToSquad(AttackSquadPtr attackSquad);

	SquadPtr mAlliedSquadCommand;
	SquadManager* mSquadManager;
	UnitManager* mUnitManager;
	BuildPlanner* mBuildPlanner;
	const UnitCompositionFactory* mUnitCompositionFactory;
	const PlayerArmyManager* mAlliedArmyManager;
	const DefenseManager* mDefenseManager;
	IntentionWriter* mIntentionWriter;
	const SelfClassifier* mSelfClassifier;
	const AlliedClassifier* mAlliedClassifier;
	const GameTime* mGameTime;

	std::map<std::string, Commands> mCommandStringToEnums;

	double mExpansionTimeLast;
	int mFrameCallLast;

	static Commander* msInstance;
};
}