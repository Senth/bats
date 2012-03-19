#pragma once

#include <vector>

// Namespace for the project
namespace bats {

// Forward declarations
class Squad;
class SquadManager;
class UnitCompositionFactory;

/**
 * The commander creates squads and sends them out to various locations. The squads are
 * created through commands and can both be created by the teammate player and by the
 * Commander itself. The commander always evaluates the teammate player's commands to ensure
 * that they are valid and can be carried out.
 * 
 * The commander also prints out information messages what this AI is up to, when issuing commands
 * or if they are invalid, in that case why.
 * 
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class Commander {
public:
	/**
	 * Destructor
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
	void computeActions();

	/**
	 * Tries to issue a specified command. This is essentially creating a new squad
	 * or adding a new goal to an existing squad.
	 * @param command the command to try to issue.
	 * @return true if the command will be issued, false otherwise.
	 * @note The command will not be issued immediately; although it will assemble the
	 * squad (if it can). It will first wait for a ping for x seconds (specified in)
	 * BATS-data\config.ini, if a ping has been assigned it will then wait for a second ping
	 * for y seconds. If a new command is specified it will complete the current command.
	 */
	bool issueCommand(const std::string& command);
	
private:
	/**
	 * Private constructor to enforce singleton pattern.
	 */
	Commander();

	/**
	 * Finishes and executes the squad command.
	 */
	void finishWaitingSquad();

	Squad* mpSquadWaiting;
	SquadManager* mpSquadManager;
	UnitCompositionFactory* mpUnitCompositionFactory;

	static Commander* mpsInstance;
};
}