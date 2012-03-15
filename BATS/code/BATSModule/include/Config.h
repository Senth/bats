#pragma once

#include <string>

namespace bats {
	namespace config {
		extern const std::string ROOT_DIR;
		extern const std::string CONFIG_DIR;

		namespace squad {
			extern const std::string UNIT_COMPOSITION_DIR;
			/** How long shall the Commander wait for the first ping */
			extern float PING_WAIT_TIME_FIRST;
			/** How long shall the Commander wait for pings after the first one */
			extern float PING_WAIT_TIME_AFTER_FIRST;
		}

		namespace log {
			extern const std::string OUTPUT_DIR;
			extern const std::string SETTINGS_FILE;
		}

		/**
		 * Loads settings from CONFIG_DIR\config.ini.
		 */
		void loadConfig();
	}
}