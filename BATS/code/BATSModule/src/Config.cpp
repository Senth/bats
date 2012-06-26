#include "Config.h"

#include "Utilities/IniReader.h"
#include "Utilities/Logger.h"
#include <map>
#include <set>
#include <list>
#include <sstream>

namespace bats {
namespace config {

//--------- Listeners ---------//
using std::string;
using std::map;

map<ConstantName, std::set<OnConstantChangedListener*>> gListeners;

void addOnConstantChangedListener(
	ConstantName constantName,
	OnConstantChangedListener* pListener)
{
	gListeners[constantName].insert(pListener);
}

void removeOnConstantChangedListener(
	ConstantName constantName,
	OnConstantChangedListener* pListener)
{
	gListeners[constantName].erase(pListener);
}

void triggerOnConstantChanged(ConstantName constantName) {
	std::set<OnConstantChangedListener*>& listeners = gListeners[constantName];
	std::set<OnConstantChangedListener*>::const_iterator it;
	for (it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onConstantChanged(constantName);
	}
}

template <typename T>
string toString(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

std::string gOldValue;
std::list<ConstantName> gTriggerQueue;

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

namespace classification {
	namespace squad {
		size_t MEASURE_TIME = 5;
		int MOVED_TILES_MIN = 0;
		int MOVED_TILES_MIN_SQUARED = MOVED_TILES_MIN * MOVED_TILES_MIN;
		double ATTACK_FRACTION_AWAY_MIN = 0.0;
		double RETREAT_FRACTION_AWAY_MIN = 0.0;
		int INCLUDE_DISTANCE = 0;
		int INCLUDE_DISTANCE_SQUARED = INCLUDE_DISTANCE * INCLUDE_DISTANCE;
		int EXCLUDE_DISTANCE = 0;
		int EXCLUDE_DISTANCE_SQUARED = EXCLUDE_DISTANCE * EXCLUDE_DISTANCE;
		int GRID_SQUARE_DISTANCE = 0;

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
	int ALLIED_ARMY_REARRANGE_SQUADS = 45;

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
		double ATTACK_TIMEOUT = 0.0;
		double LOAD_TIMEOUT = 0.0;

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
	gOldValue = "";
	gTriggerQueue.clear();

	bool success = true;
	if (variableInfo.section == "attack_coordinator") {
		success = attack_coordinator::set(variableInfo);
	} else if (variableInfo.section == "classification") {
		success = classification::set(variableInfo);
	} else if (variableInfo.section == "debug") {
		success = debug::set(variableInfo);
	} else if (variableInfo.section == "frame_distribution") {
		success = frame_distribution::set(variableInfo);
	} else if (variableInfo.section == "game") {
		success = game::set(variableInfo);
	} else if (variableInfo.section == "squad") {
		success = squad::set(variableInfo);
	} else {
		ERROR_MESSAGE(false, "Unknown section [" << variableInfo.section
			<< "] in " << variableInfo.file << ".ini");
	}

	
	if (success) {
		// Value changed
		if (gOldValue != static_cast<string>(variableInfo)) {
			// Trigger all triggers
			while (!gTriggerQueue.empty()) {
				triggerOnConstantChanged(gTriggerQueue.front());
				gTriggerQueue.pop_front();
			}
		}
	} else {
		if (variableInfo.subsection.empty()) {
			ERROR_MESSAGE(false, "Unkown variable name '" << variableInfo.name
				<< "' in " << variableInfo.file << ".ini, [" << variableInfo.section << "]!");
		} else {
			ERROR_MESSAGE(false, "Unkown variable name '" << variableInfo.name
				<< "' in " << variableInfo.file << ".ini, [" << variableInfo.section << "." <<
				variableInfo.subsection << "]!");
		}
	}
}

bool attack_coordinator::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.subsection == "weights") {
		return weights::set(variableInfo);
	} else if (variableInfo.subsection.empty()) {
		if (variableInfo.name == "expansion_not_checked_time") {
			gOldValue = toString(EXPANSION_NOT_CHECKED_TIME);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(EXPANSION_NOT_CHECKED_TIME));
			EXPANSION_NOT_CHECKED_TIME = variableInfo;
		} else if (variableInfo.name == "wait_goal_timeout") {
			gOldValue = toString(WAIT_GOAL_TIMEOUT);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(WAIT_GOAL_TIMEOUT));
			WAIT_GOAL_TIMEOUT = variableInfo;
		} else {
			return false;
		} 
	} else {
		ERROR_MESSAGE(false, "Unkown subsection '" << variableInfo.subsection <<
			"' in " << variableInfo.file << ".ini");
	}

	return true;
}

bool attack_coordinator::weights::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "expansion_not_checked") {
		gOldValue = toString(EXPANSION_NOT_CHECKED);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(EXPANSION_NOT_CHECKED));
		EXPANSION_NOT_CHECKED = variableInfo;
	} else if (variableInfo.name == "expansion_max") {
		gOldValue = toString(EXPANSION_MAX);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(EXPANSION_MAX));
		EXPANSION_MAX = variableInfo;
	} else if (variableInfo.name == "expansion_min") {
		gOldValue = toString(EXPANSION_MIN);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(EXPANSION_MIN));
		EXPANSION_MIN = variableInfo;
	} else if (variableInfo.name == "expansion_ceil") {
		gOldValue = toString(EXPANSION_CEIL);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(EXPANSION_CEIL));
		EXPANSION_CEIL = variableInfo;
	} else if (variableInfo.name == "addon_structure") {
		gOldValue = toString(ADDON_STRUCTURE);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(ADDON_STRUCTURE));
		ADDON_STRUCTURE = variableInfo;
	} else if (variableInfo.name == "supply_structure") {
		gOldValue = toString(SUPPLY_STRUCTURE);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(SUPPLY_STRUCTURE));
		SUPPLY_STRUCTURE = variableInfo;
	} else if (variableInfo.name == "upgrade_structure") {
		gOldValue = toString(UPGRADE_STRUCTURE);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(UPGRADE_STRUCTURE));
		UPGRADE_STRUCTURE = variableInfo;
	} else if (variableInfo.name == "unit_producing_structure") {
		gOldValue = toString(UNIT_PRODUCING_STRUCTURE);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(UNIT_PRODUCING_STRUCTURE));
		UNIT_PRODUCING_STRUCTURE = variableInfo;
	} else if (variableInfo.name == "other_structure") {
		gOldValue = toString(OTHER_STRUCTURE);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(OTHER_STRUCTURE));
		OTHER_STRUCTURE = variableInfo;
	} else {
		return false;
	}

	return true;
}

bool classification::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.subsection == "squad") {
		return squad::set(variableInfo);
	} else if (variableInfo.subsection.empty()) {
		return false;
	} else {
		ERROR_MESSAGE(false, "Unkown subsection '" << variableInfo.subsection <<
			"' in " <<variableInfo.file << ".ini");
	}

	return true;
}

bool classification::squad::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "measure_time") {
		gOldValue = toString(MEASURE_TIME);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(MEASURE_TIME));
		MEASURE_TIME = variableInfo;
	} else if (variableInfo.name == "moved_tiles_min") {
		gOldValue = toString(MOVED_TILES_MIN);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(MOVED_TILES_MIN));
		gTriggerQueue.push_back(TO_CONSTANT_NAME(MOVED_TILES_MIN_SQUARED));
		MOVED_TILES_MIN = variableInfo;
		MOVED_TILES_MIN_SQUARED = MOVED_TILES_MIN * MOVED_TILES_MIN;
	} else if (variableInfo.name == "attack_fraction_away_min") {
		gOldValue = toString(ATTACK_FRACTION_AWAY_MIN);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(ATTACK_FRACTION_AWAY_MIN));
		ATTACK_FRACTION_AWAY_MIN = variableInfo;
	} else if (variableInfo.name == "retreat_fraction_away_min") {
		gOldValue = toString(RETREAT_FRACTION_AWAY_MIN);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(RETREAT_FRACTION_AWAY_MIN));
		RETREAT_FRACTION_AWAY_MIN = variableInfo;
	} else if (variableInfo.name == "include_distance") {
		gOldValue = toString(INCLUDE_DISTANCE);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(INCLUDE_DISTANCE));
		gTriggerQueue.push_back(TO_CONSTANT_NAME(INCLUDE_DISTANCE_SQUARED));
		INCLUDE_DISTANCE = variableInfo;
		INCLUDE_DISTANCE_SQUARED = INCLUDE_DISTANCE * INCLUDE_DISTANCE;
	} else if (variableInfo.name == "exclude_distance") {
		gOldValue = toString(EXCLUDE_DISTANCE);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(EXCLUDE_DISTANCE));
		gTriggerQueue.push_back(TO_CONSTANT_NAME(EXCLUDE_DISTANCE_SQUARED));
		gTriggerQueue.push_back(TO_CONSTANT_NAME(GRID_SQUARE_DISTANCE));
		EXCLUDE_DISTANCE = variableInfo;
		EXCLUDE_DISTANCE_SQUARED = EXCLUDE_DISTANCE * EXCLUDE_DISTANCE;
		GRID_SQUARE_DISTANCE = EXCLUDE_DISTANCE / 2;
	} else  {
		return false;
	}

	return true;
}

bool debug::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "graphics_text_verbosity_in_debug") {
		gOldValue = toString(GRAPHICS_TEXT_VERBOSITY_IN_DEBUG);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(GRAPHICS_TEXT_VERBOSITY_IN_DEBUG));
		GRAPHICS_TEXT_VERBOSITY_IN_DEBUG = variableInfo;
	} else if (variableInfo.name == "graphics_text_verbosity_in_release") {
		gOldValue = toString(GRAPHICS_TEXT_VERBOSITY_IN_RELEASE); gTriggerQueue.push_back(TO_CONSTANT_NAME(GRAPHICS_TEXT_VERBOSITY_IN_RELEASE));
		GRAPHICS_TEXT_VERBOSITY_IN_RELEASE = variableInfo;
	} else {
		return false;
	}

	return true;
}

bool frame_distribution::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "exploration_manager") {
		gOldValue = toString(EXPLORATION_MANAGER);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(EXPLORATION_MANAGER));
		EXPLORATION_MANAGER = variableInfo;
	} else if (variableInfo.name == "resource_counter") {
		gOldValue = toString(RESOURCE_COUNTER);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(RESOURCE_COUNTER));
		RESOURCE_COUNTER = variableInfo;
	} else if (variableInfo.name == "allied_army_rearrange_squads") {
		gOldValue = toString(ALLIED_ARMY_REARRANGE_SQUADS);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(ALLIED_ARMY_REARRANGE_SQUADS));
		ALLIED_ARMY_REARRANGE_SQUADS = variableInfo;
	} else {
		return false;
	}

	return true;
}

bool game::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "speed") {
		gOldValue = toString(SPEED);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(SPEED));
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
			gOldValue = toString(PING_WAIT_TIME_FIRST);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(PING_WAIT_TIME_FIRST));
			PING_WAIT_TIME_FIRST = variableInfo;
		} else if (variableInfo.name == "ping_wait_time_after_first") {
			gOldValue = toString(PING_WAIT_TIME_AFTER_FIRST);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(PING_WAIT_TIME_AFTER_FIRST));
			PING_WAIT_TIME_AFTER_FIRST = variableInfo;
		} else if (variableInfo.name == "regroup_distance_begin") {
			gOldValue = toString(REGROUP_DISTANCE_BEGIN);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(REGROUP_DISTANCE_BEGIN));
			gTriggerQueue.push_back(TO_CONSTANT_NAME(REGROUP_DISTANCE_BEGIN_SQUARED));
			REGROUP_DISTANCE_BEGIN = variableInfo;
			REGROUP_DISTANCE_BEGIN_SQUARED = REGROUP_DISTANCE_BEGIN * REGROUP_DISTANCE_BEGIN;
		} else if (variableInfo.name == "regroup_distance_end") {
			gOldValue = toString(REGROUP_DISTANCE_END);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(REGROUP_DISTANCE_END));
			gTriggerQueue.push_back(TO_CONSTANT_NAME(REGROUP_DISTANCE_END_SQUARED));
			REGROUP_DISTANCE_END = variableInfo;
			REGROUP_DISTANCE_END_SQUARED = REGROUP_DISTANCE_END * REGROUP_DISTANCE_END;
		} else if (variableInfo.name == "regroup_new_position_time") {
			gOldValue = toString(REGROUP_NEW_POSITION_TIME);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(REGROUP_NEW_POSITION_TIME));
			REGROUP_NEW_POSITION_TIME = variableInfo;
		} else if (variableInfo.name == "calc_furthest_away_time") {
			gOldValue = toString(CALC_FURTHEST_AWAY_TIME);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(CALC_FURTHEST_AWAY_TIME));
			CALC_FURTHEST_AWAY_TIME = variableInfo;
		} else if (variableInfo.name == "close_distance") {
			gOldValue = toString(CLOSE_DISTANCE);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(CLOSE_DISTANCE));
			CLOSE_DISTANCE = variableInfo;
		} else if (variableInfo.name == "sight_distance_multiplier") {
			gOldValue = toString(SIGHT_DISTANCE_MULTIPLIER);
			gTriggerQueue.push_back(TO_CONSTANT_NAME(SIGHT_DISTANCE_MULTIPLIER));
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
		gOldValue = toString(WAITING_POSITION_DISTANCE_FROM_GOAL);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(WAITING_POSITION_DISTANCE_FROM_GOAL));
		WAITING_POSITION_DISTANCE_FROM_GOAL = variableInfo;
	} else if (variableInfo.name == "structures_destroyed_goal_distance") {
		gOldValue = toString(STRUCTURES_DESTROYED_GOAL_DISTANCE);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(STRUCTURES_DESTROYED_GOAL_DISTANCE));
		STRUCTURES_DESTROYED_GOAL_DISTANCE = variableInfo;
	} else {
		return false;
	}
	
	return true;
}

bool squad::drop::set(const utilities::VariableInfo& variableInfo) {
	if (variableInfo.name == "attack_timeout") {
		gOldValue = toString(ATTACK_TIMEOUT);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(ATTACK_TIMEOUT));
		ATTACK_TIMEOUT = variableInfo;
	} else if (variableInfo.name == "load_timeout") {
		gOldValue = toString(LOAD_TIMEOUT);
		gTriggerQueue.push_back(TO_CONSTANT_NAME(LOAD_TIMEOUT));
		LOAD_TIMEOUT = variableInfo;
	} else {
		return false;
	}

	return true;
}

}
}