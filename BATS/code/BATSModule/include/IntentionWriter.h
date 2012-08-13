#pragma once

#include "IntentionMessage.h"
#include <vector>
#include <map>
#include <BWAPI/TilePosition.h>

// Forward declaration
namespace utilities {
	struct VariableInfo;
}


// Namespace for the project
namespace bats {

/**
 * All the different types of intentions
 */
enum Intentions {
	Intention_AlliedAttackFollow, /**< Will follow the allied attacking squad */
	Intention_BotAttack, /**< A regular attack by the enemy */
	Intention_BotAttackMerged, /**< Merged new units with the old squad */
	Intention_BotAttackNewPosition, /**< Finding a new position to attack */
	Intention_BotAttackNot, /**< It's not possible to create another attack */
	Intention_BotComingToAid, /**< The bot is coming to aid the player in defending */
	Intention_BotComingToAidNot, /**< Bot will not help the allied with defending */
	Intention_BotDrop, /**< Does a drop */
	Intention_BotDropNewPosition, /**< Finding a new position to drop */
	Intention_BotDropNot, /**< Can't create a drop */
	Intention_BotExpand, /**< The bot is about to expand */
	Intention_BotExpandNot, /**< Can't create an expansion */
	Intention_BotRetreat, /**< The bot retreats from a battle */
	Intention_BotScout, /**< Creates a scout */
	Intention_BotScoutNot, /**< Could not create a scout */
	Intention_WeShouldRetreat, /**< Good if we retreat */

	Intention_Lim /**< Invalid intention */
};

/**
 * Reasons for the intentions
 */
enum Reasons {
	Reason_AlliedExpanding, /**< The allied started to build an expansion */
	Reason_AlliedMovingToAttack, /**< Allied squad is moving out to attack */
	Reason_BotAttackSuccess, /**< The attack finished successfully */
	Reason_BotDidNotAttack, /**< Did not attack anything */
	Reason_BotDropTimedOut, /**< Drop timed out */
	Reason_BotIsUnderAttack, /**< Bot is under attack */
	Reason_BotNotEnoughUnits, /**< Bot does not have enough units for the action */
	Reason_BotTooManyAttacks, /**< The bot has too many attacks for the action */
	Reason_EnemyTooStrong, /**< Enemy force is too strong */

	Reason_Lim /**< Invalid reason */
};

/**
 * Types out the specific intention to allied players, but only if enough time has passed. The time
 * is determined in messages.ini file for each specific type of intention. The default
 * time is set under the [config] section, i.e. not in config.ini.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class IntentionWriter {
public:
	/**
	 * Destructor
	 */
	virtual ~IntentionWriter();

	/**
	 * Returns the instance of IntentionWriter.
	 * @return instance of IntentionWriter.
	 */
	static IntentionWriter* getInstance();

	/**
	 * Writes out the specified intention, but only if it enough time has passed. The time
	 * is determined in messages.ini file for each specific type of intention. The default
	 * time is set under the [config] section, i.e. not in config.ini.
	 * It can also pass an optional parameter to ping in that location.
	 * @param intention the intention to write out.
	 * @param reason an optional reason for the intention to write out, defaults to Reasons_Lim.
	 * @param pingLocation optional location to ping on the map, defaults to TilePositions::Invalid.
	 */
	void writeIntention(
		Intentions intention,
		Reasons reason = Reason_Lim,
		const BWAPI::TilePosition& pingLocation = BWAPI::TilePositions::Invalid
	);

	/**
	 * Reloads the configuration files with all intentions and reasons
	 */
	void reloadConfig();

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	IntentionWriter();

	/**
	 * Initialize the string to enum wrappers for intentions and reasons.
	 */
	void initStringToEnumWrappers();

	/**
	 * Handle intention config variable
	 * @param variableInfo the configuration file variable
	 */
	void handleIntentionVariable(const utilities::VariableInfo& variableInfo);

	/**
	 * Handle reason config variable
	 * @param variableInfo the configuration file variable
	 */
	void handleReasonVariable(const utilities::VariableInfo& variableInfo);

	std::vector<Message> mReasons;
	std::vector<IntentionMessage> mIntentions;

	// Wrappers for string to enum, used when reading config file
	std::map<std::string, Intentions> mIntentionStringToEnums;
	std::map<std::string, Reasons> mReasonStringToEnums;

	static IntentionWriter* msInstance;
};
}