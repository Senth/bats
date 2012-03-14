#include "Config.h"

namespace bats {
namespace config {
	const std::string ROOT_DIR = "bwapi-data\\";
	const std::string CONFIG_DIR = ROOT_DIR + "AI\\BATS-data\\";
	const std::string UNIT_COMPOSITION_DIR = CONFIG_DIR + "UnitComposition";

	namespace log {
		const std::string SETTINGS_FILE = CONFIG_DIR + "log_settings.ini";
		const std::string LOGGING_PATH = ROOT_DIR + "logs\\bats";
	}
}
}