#include "Config.h"

#include "Utilities/IniReader.h"
#include "Utilities/Logger.h"


namespace bats {
namespace config {

//--------- Constants ---------//
const std::string ROOT_DIR = "bwapi-data\\";
const std::string CONFIG_DIR = ROOT_DIR + "AI\\BATS-data\\";

namespace attack_coordinator {
	double EXPANSION_NOT_CHECKED_TIME = 60.0;
	double WAIT_GOAL_TIMEOUT = 30.0;

	namespace weights {
		double EXPANSION_NOT_CHECKED = 1.0;
		double EXPANSION_MAX = 1.0;
		double EXPANSION_MIN = 0.0;
		bool EXPANSION_CEIL = true;
		double ADDON_STRUCTURE = 1.0;
		double SUPPLY_STRUCTURE = 1.0;
		double UPGRADE_STRUCTURE = 1.0;
		double UNIT_PRODUCING_STRUCTURE = 1.0;
		double OTHER_STRUCTURE = 1.0;

		bool set(const utilities::VariableInfo& variableInfo);
	}

	bool set(const utilities::VariableInfo& variableInfo);
}

namespace debug {
	int GRAPHICS_TEXT_VERBOSITY_IN_DEBUG = 0;
	int GRAPHICS_TEXT_VERBOSITY_IN_RELEASE = 0;
	int DEBUG_MESSAGE_VERBOSITY = 0;

	bool set(const utilities::VariableInfo& variableInfo);
}

namespace build_order {
	const std::string DIR = CONFIG_DIR + "buildorder\\";
}

namespace frame_distribution {
	int EXPLORATION_MANAGER = 61;
	int RESOURCE_COUNTER = 23;

	bool set(const utilities::VariableInfo& variableInfo);
}

namespace game {
	int SPEED = 8;

	bool set(const utilities::VariableInfo& variableInfo);
}

namespace log {
	const std::string SETTINGS_FILE = CONFIG_DIR + "log_settings.ini";
	const std::string OUTPUT_DIR = ROOT_DIR + "logs\\bats";
}

namespace squad {
	const std::string UNIT_COMPOSITION_DIR = CONFIG_DIR + "UnitCompositions";
	double PING_WAIT_TIME_FIRST = 0.0;
	double PING_WAIT_TIME_AFTER_FIRST = 0.0;
	double REGROUP_DISTANCE_BEGIN = 0.0;
	double REGROUP_DISTANCE_BEGIN_SQUARED = REGROUP_DISTANCE_BEGIN * REGROUP_DISTANCE_BEGIN;
	double REGROUP_DISTANCE_END = 0.0;
	double REGROUP_DISTANCE_END_SQUARED = REGROUP_DISTANCE_END * REGROUP_DISTANCE_END;
	double REGROUP_NEW_POSITION_TIME = 0.0;
	double CALC_FURTHEST_AWAY_TIME = 1.0;
	double CLOSE_DISTANCE = 0.0;
	double SIGHT_DISTANCE_MULTIPLIER;

	namespace attack {
		double WAITING_POSITION_DISTANCE_FROM_GOAL = 0.0;
		double STRUCTURES_DESTROYED_GOAL_DISTANCE = 0.0;

		bool set(const utilities::VariableInfo& variableInfo);
	}

	namespace drop {
		double TIMEOUT = 0.0;

		bool set(const utilities::VariableInfo& variableInfo);
	}

	bool set(const utilities::VariableInfo& variableInfo);
}

namespace wait_goals {
	const std::string ATTACK_COORDINATION = "attack_coordinaton";
}


//--------- Function Definitions ---------//
void readConfig(const std::string& configFile);
void handleVariable(const utilities::VariableInfo& variableInfo);


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
	if (variableInfo.section == "attack_coordinator") {
		noErrors = attack_coordinator::set(variableInfo);
	} else if (variableInfo.section == "debug") {
		noErrors = debug::set(variableInfo);
	} else if (variableInfo.section == "frame_distribution") {
		noErrors = frame_distribution::set(variableInfo);
	} else if (variableInfo.section == "game") {
		noErrors = game::set(variableInfo);	
	} else if (variableInfo.section == "squad") {
		noErrors = squad::set(variableInfo);
	} else {
		ERROR_MESSAGE(false, "Unknown section [" << variableInfo.section
			<< "] in " << variableInfo.file << ".ini");
	}

	if (!noErrors) {
		ERROR_MESSAGE(false, "Unkown variable name '" << variableInfo.name
			<< "' in " << variableInfo.file << ".ini, [" << variableInfo.section << "]!");
	}
}

bool attack_coordinator::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.subsection == "weights") {
		return weights::set(variableInfo);
	} else if (variableInfo.subsection.empty()) {
		if (variableInfo.name == "expansion_not_checked_time") {
			EXPANSION_NOT_CHECKED_TIME = variableInfo;
		} else if (variableInfo.name == "wait_goal_timeout") {
			WAIT_GOAL_TIMEOUT = variableInfo;
		} else {
			return false;
		} 
	} else {
		ERROR_MESSAGE(false, "Unkown subsection '" << variableInfo.subsection <<
			"' in " <<variableInfo.file << ".ini");
	}

	return true;
}

bool attack_coordinator::weights::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "expansion_not_checked") {
		EXPANSION_NOT_CHECKED = variableInfo;
	} else if (variableInfo.name == "expansion_max") {
		EXPANSION_MAX = variableInfo;
	} else if (variableInfo.name == "expansion_min") {
		EXPANSION_MIN = variableInfo;
	} else if (variableInfo.name == "expansion_ceil") {
		EXPANSION_CEIL = variableInfo;
	} else if (variableInfo.name == "addon_structure") {
		ADDON_STRUCTURE = variableInfo;
	} else if (variableInfo.name == "supply_structure") {
		SUPPLY_STRUCTURE = variableInfo;
	} else if (variableInfo.name == "upgrade_structure") {
		UPGRADE_STRUCTURE = variableInfo;
	} else if (variableInfo.name == "unit_producing_structure") {
		UNIT_PRODUCING_STRUCTURE = variableInfo;
	} else if (variableInfo.name == "other_structure") {
		OTHER_STRUCTURE = variableInfo;
	} else {
		return false;
	}

	return true;
}

bool debug::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "graphics_text_verbosity_in_debug") {
		GRAPHICS_TEXT_VERBOSITY_IN_DEBUG = variableInfo;
	} else if (variableInfo.name == "graphics_text_verbosity_in_release") {
		GRAPHICS_TEXT_VERBOSITY_IN_RELEASE = variableInfo;
	} else {
		return false;
	}

	return true;
}

bool frame_distribution::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "exploration_manager") {
		EXPLORATION_MANAGER = variableInfo;
	} else if (variableInfo.name == "resource_counter") {
		RESOURCE_COUNTER = variableInfo;
	} else {
		return false;
	}

	return true;
}

bool game::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "speed") {
		SPEED = variableInfo;
	} else {
		return false;
	}

	return true;
}

bool squad::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.subsection == "attack") {
		return attack::set(variableInfo);
	} else if (variableInfo.subsection == "drop") {
		return drop::set(variableInfo);
	} else if (variableInfo.subsection.empty()) {
		if (variableInfo.name == "ping_wait_time_first") {
			PING_WAIT_TIME_FIRST = variableInfo;
		} else if (variableInfo.name == "ping_wait_time_after_first") {
			PING_WAIT_TIME_AFTER_FIRST = variableInfo;
		} else if (variableInfo.name == "regroup_distance_begin") {
			REGROUP_DISTANCE_BEGIN = variableInfo;
			REGROUP_DISTANCE_BEGIN_SQUARED = REGROUP_DISTANCE_BEGIN * REGROUP_DISTANCE_BEGIN;
		} else if (variableInfo.name == "regroup_distance_end") {
			REGROUP_DISTANCE_END = variableInfo;
			REGROUP_DISTANCE_END_SQUARED = REGROUP_DISTANCE_END * REGROUP_DISTANCE_END;
		} else if (variableInfo.name == "regroup_new_position_time") {
			REGROUP_NEW_POSITION_TIME = variableInfo;
		} else if (variableInfo.name == "calc_furthest_away_time") {
			CALC_FURTHEST_AWAY_TIME = variableInfo;
		} else if (variableInfo.name == "close_distance") {
			CLOSE_DISTANCE = variableInfo;
		} else if (variableInfo.name == "sight_distance_multiplier") {
			SIGHT_DISTANCE_MULTIPLIER = variableInfo;
		} else {
			return false;
		}
	} else {
		ERROR_MESSAGE(false, "Unknown subsection '" << variableInfo.section << "." <<
			variableInfo.subsection << "' in " << variableInfo.file << ".ini");
	}

	return true;
}

bool squad::attack::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "waiting_position_distance_from_goal") {
		WAITING_POSITION_DISTANCE_FROM_GOAL = variableInfo;
	} else if (variableInfo.name == "structures_destroyed_goal_distance") {
		STRUCTURES_DESTROYED_GOAL_DISTANCE = variableInfo;
	} else {
		return false;
	}
	
	return true;
}

bool squad::drop::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "timeout") {
		TIMEOUT = variableInfo;
	} else {
		return false;
	}

	return true;
}

}
}