#pragma once

#include <string>

namespace bats {
	namespace config {
		extern const std::string ROOT_DIR;
		extern const std::string CONFIG_DIR;

		namespace frame_distribution {
			extern int EXPLORATION_MANAGER;
		}

		namespace game {
			extern int SPEED;
		}

		namespace log {
			extern const std::string OUTPUT_DIR;
			extern const std::string SETTINGS_FILE;
		}

		namespace squad {
			extern const std::string UNIT_COMPOSITION_DIR;
			/** How long shall the Commander wait for the first ping */
			extern float PING_WAIT_TIME_FIRST;
			/** How long shall the Commander wait for pings after the first one */
			extern float PING_WAIT_TIME_AFTER_FIRST;
			/** Decides the distance away from the center a unit has to be until a group occurs.
			 * If a unit is further away from each other than this distance from the squads
			 * center then the squads will get a temporary goal to center of the squad to
			 * regroup. 
			 * @see REGROUP_DISTANCE_END for when regrouping shall end. These two variables
			 * should not have the same value since this can cause threshold errors.
			 */
			extern float REGROUP_DISTANCE_BEGIN;
			/** Decides the distance away from the center all units has to be until a regroup
			 * is considered to be done.
			 * @see REGROUP_DISTANCE_BEGIN for the distance when regrouping shall begin.
			 */
			extern float REGROUP_DISTANCE_END;
		}

		/**
		 * Loads settings from CONFIG_DIR\config.ini.
		 */
		void loadConfig();
	}
}