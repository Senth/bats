#include "IntentionWriter.h"
#include "Config.h"
#include "Utilities/Logger.h"
#include "Utilities/IniReader.h"
#include <cstdlib> // For NULL
#include <algorithm>
#include <BWAPI/Game.h>

using namespace bats;
using namespace BWAPI;
using namespace std;

IntentionWriter* IntentionWriter::msInstance = NULL;

const std::string CONFIG_FILE = "messages.ini";

IntentionWriter::IntentionWriter() {
	initStringToEnumWrappers();
	reloadConfig();
}

IntentionWriter::~IntentionWriter() {
	msInstance = NULL;
}

IntentionWriter* IntentionWriter::getInstance() {
	if (NULL == msInstance) {
		msInstance = new IntentionWriter();
	}
	return msInstance;
}

void IntentionWriter::writeIntention(Intentions intention, Reasons reason, const TilePosition& pingLocation) {
	// Write intention only if the module is active
	if (config::module::WRITE_INTENTION == false) {
		return;
	}

	const string& intentionMessage = mIntentions[intention].getMessage();
	
	// Empty can't send that message yet
	if (intentionMessage == "") {
		return;
	}


	// Capitalize first letter in sentence.
	string fullMessage = intentionMessage;
	std::transform(fullMessage.begin(), fullMessage.begin()+1, fullMessage.begin(), ::toupper);


	// Get the reason
	const string& reasonMessage = mReasons[reason].getMessage();

	// Append reason to full message
	if (reasonMessage != "") {
		fullMessage += ", ";
		fullMessage += reasonMessage;
	}
	fullMessage += ".";

	DEBUG_MESSAGE(utilities::LogLevel_Finest, "WriteIntention::writeIntention(): " <<
		mIntentions[intention].getName() << "—" << mReasons[reason].getName() << " -> " << fullMessage
	);
	Broodwar->sendTextEx(true, "%s", fullMessage.c_str());


	// Ping the location
	if (pingLocation.isValid()) {
		Position ping(pingLocation);
		Broodwar->pingMinimap(ping.x(), ping.y());
	}
}

void IntentionWriter::reloadConfig() {
	utilities::IniReader configReader;
	string configFile = config::CONFIG_DIR + CONFIG_FILE;
	configReader.open(configFile);

	if (!configReader.isOpen()) {
		ERROR_MESSAGE(false, "IntentionWriter: Could not open config file: " << configFile);
	}

	mReasons.clear();
	mReasons.resize(Reason_Lim+1);
	mIntentions.clear();
	mIntentions.resize(Intention_Lim+1);
	IntentionMessage::setDefaultIntervalTimeMin(0.0);

	utilities::VariableInfo variableInfo;
	while (configReader.isGood()) {
		configReader.readNext(variableInfo);

		if (variableInfo.section == "config") {
			if (variableInfo.name == "default_intention_interval_min") {
				IntentionMessage::setDefaultIntervalTimeMin(variableInfo);
			} else {
				ERROR_MESSAGE(false, "IntentionWriter: Unknown variable name '" <<
					variableInfo.name << "' in section: " << variableInfo.section
				);
			}
		} else if (variableInfo.section == "intention") {
			handleIntentionVariable(variableInfo);
		} else if (variableInfo.section == "reason") {
			handleReasonVariable(variableInfo);
		} else  {
			ERROR_MESSAGE(false, "IntentionWriter: Unknown section: " << variableInfo.section);
		}
	}
}

void IntentionWriter::handleIntentionVariable(const utilities::VariableInfo& variableInfo) {
	map<string, Intentions>::iterator foundIt = mIntentionStringToEnums.find(variableInfo.subsection);
	
	if (foundIt != mIntentionStringToEnums.end()) {
		IntentionMessage& currentIntention = mIntentions[foundIt->second];

		// Set name, although quite ineffective to set each time, this is only called when reloading the file
		currentIntention.setName(variableInfo.subsection);

		if (variableInfo.name == "message") {
			currentIntention.addMessage(variableInfo);
		} else if (variableInfo.name == "interval_min") {
			currentIntention.setIntervalTimeMin(variableInfo);
		} else {
			ERROR_MESSAGE(false, "InentionWriter: Unknown variable name '" << variableInfo.name <<
				"' for [" << variableInfo.section << "." << variableInfo.subsection << "]"
			);
		}
	} else {
		ERROR_MESSAGE(false, "IntentionWriter: Unknown intention type (subsection): " << variableInfo.subsection);
	}
}

void IntentionWriter::handleReasonVariable(const utilities::VariableInfo& variableInfo) {
	map<string, Reasons>::iterator foundIt = mReasonStringToEnums.find(variableInfo.subsection);

	if (foundIt != mReasonStringToEnums.end()) {
		Message& currentReason = mReasons[foundIt->second];

		// Set name, although quite ineffective to set each time, this is only called when reloading the file
		currentReason.setName(variableInfo.subsection);

		if (variableInfo.name == "message") {
			currentReason.addMessage(variableInfo);
		} else {
			ERROR_MESSAGE(false, "InentionWriter: Unknown variable name '" << variableInfo.name <<
				"' for [" << variableInfo.section << "." << variableInfo.subsection << "]"
				);
		}
	} else {
		ERROR_MESSAGE(false, "IntentionWriter: Unknown intention type (subsection): " << variableInfo.subsection);
	}
}

void IntentionWriter::initStringToEnumWrappers() {
	// Intentions
	mIntentionStringToEnums["AlliedAttackFollow"] = Intention_AlliedAttackFollow;
	mIntentionStringToEnums["BotAttack"] = Intention_BotAttack;
	mIntentionStringToEnums["BotAttackNot"] = Intention_BotAttackNot;
	mIntentionStringToEnums["BotComingToAid"] = Intention_BotComingToAid;
	mIntentionStringToEnums["BotComingToAidNot"] = Intention_BotComingToAidNot;
	mIntentionStringToEnums["BotDrop"] = Intention_BotDrop;
	mIntentionStringToEnums["BotRetreat"] = Intention_BotRetreat;
	mIntentionStringToEnums["BotExpand"] = Intention_BotExpand;
	
	ERROR_MESSAGE_CONDITION(mIntentionStringToEnums.size() != Intention_Lim,
		false,
		"IntentionWriter: Intentions enum does not have same size as mIntentionStringToEnums!"
	);


	// Reasons
	mReasonStringToEnums["AlliedExpanding"] = Reason_AlliedExpanding;
	mReasonStringToEnums["AlliedMovingToAttack"] = Reason_AlliedMovingToAttack;
	mReasonStringToEnums["BotNotEnoughUnits"] = Reason_BotNotEnoughUnits;
	mReasonStringToEnums["BotTooManyAttacks"] = Reason_BotTooManyAttacks;
	mReasonStringToEnums["BotIsUnderAttack"] = Reason_BotIsUnderAttack;

	ERROR_MESSAGE_CONDITION(mReasonStringToEnums.size() != Reason_Lim,
		false,
		"IntentionWriter: Reasons enum does not have the same size as mReasonStringToEnums!"
	);
}