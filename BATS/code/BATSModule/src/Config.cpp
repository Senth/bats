#include "Config.h"

#include "Utilities/IniReader.h"
#include "Utilities/Logger.h"


namespace bats {
namespace config {

//--------- Constants ---------//
const std::string ROOT_DIR = "bwapi-data\\";
const std::string CONFIG_DIR = ROOT_DIR + "AI\\BATS-data\\";

namespace squad {
	const std::string UNIT_COMPOSITION_DIR = CONFIG_DIR + "UnitComposition";
	float PING_WAIT_TIME_FIRST = 0.0f;
	float PING_WAIT_TIME_AFTER_FIRST = 0.0f;
}

namespace log {
	const std::string SETTINGS_FILE = CONFIG_DIR + "log_settings.ini";
	const std::string OUTPUT_DIR = ROOT_DIR + "logs\\bats";
}


//--------- Function Definitions ---------//
void readConfig(const std::string& configFile);
void handleVariable(const utilities::VariableInfo& variableInfo);
bool setSquad(const utilities::VariableInfo& variableInfo);


//--------- Function Declarations ---------//
void loadConfig() {
	// Default configuration
	readConfig(CONFIG_DIR + "config_default.ini");
	// Overridden configuration
	readConfig(CONFIG_DIR + "config_override.ini");
}

void readConfig(const std::string& configFile) {
	utilities::IniReader configReader;
	configReader.open(configFile, true);

	if (!configReader.isOpen()) {
		DEBUG_MESSAGE(utilities::LogLevel_Warning, "readConfig() | Could not open configuration file: "
			<< configFile);
	}

	while (configReader.isGood()) {
		utilities::VariableInfo variableInfo;
		configReader.readNext(variableInfo);

		handleVariable(variableInfo);
	}
}

void handleVariable(const utilities::VariableInfo& variableInfo) {
	bool noErrors = true;
	if (variableInfo.section == "squad") {
		noErrors = setSquad(variableInfo);
	} else {
		ERROR_MESSAGE(false, "Unknown section '" << variableInfo.section
			<< "' in " << variableInfo.file << ".ini");
	}

	if (!noErrors) {
		ERROR_MESSAGE(false, "Unkown variable name '" << variableInfo.name
			<< "' in " << variableInfo.file << ".ini");
	}
}

bool setSquad(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "ping_wait_time_first") {
		squad::PING_WAIT_TIME_FIRST = variableInfo;
	} else if (variableInfo.name == "ping_wait_time_after_first") {
		squad::PING_WAIT_TIME_AFTER_FIRST = variableInfo;
	} else {
		return false;
	}

	return true;
}

}
}