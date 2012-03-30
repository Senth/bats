#pragma once

#include <string>

namespace bats {
	/**
	 * Configuration variables for BATS. Most of these can be configured in
	 * the config files. config_default.ini and config_override.ini
	 */
	namespace config {
		extern const std::string ROOT_DIR;
		extern const std::string CONFIG_DIR;

		/**
		 * Configuration specific for attack_coordinator
		 */
		namespace attack_coordinator {
			/** Time until an expansion shall be considered not checked. In seconds */
			extern double EXPANSION_NOT_CHECKED_TIME;
			/** Attack coordination wait goal time outs */
			extern double WAIT_GOAL_TIMEOUT;

			/**
			 * All weights the attack coordinator uses to weight and prioritize
			 * places to attack.
			 */
			namespace weights {
				/** Weight of the not checked expansions */
				extern double EXPANSION_NOT_CHECKED;
				/** The maximum weight an expansion can have. */
				extern double EXPANSION_MAX;
				/** The minimum weight an expansion can have. If this is set to something else
				 * than 0, its usage will depend on if EXPANSION_CEIL is set to true or
				 * false.
				 * 
				 * Example: If EXPANSION_MIN is set to 0.2,EXPANSION_MAX to 1.0,
				 * and EXPANSION_CEIL to true it will ceil expansions with less than 20%
				 * to a weight value of 0.2. If EXPANSION_CEIL is false and an expansion
				 * is at 20% it will now treat 20% as 20% from 0.2 to 1.0
				 * (i.e. a weight of 0.36). */
				extern double EXPANSION_MIN;
				/** Enables/Disable the use of ceil function for EXPANSION_MIN.
				 * @see EXPANSION_MIN for usage. */
				extern bool EXPANSION_CEIL;
				/** Addon weight */
				extern double ADDON_STRUCTURE;
				/** Supply weight */
				extern double SUPPLY_STRUCTURE;
				/** Upgrade weight */
				extern double UPGRADE_STRUCTURE;
				/** Unit producing structure weights */
				extern double UNIT_PRODUCING_STRUCTURE;
				/** Other structures weight */
				extern double OTHER_STRUCTURE;
			}
		}

		/**
		 * How often classes shall be called.
		 */
		namespace frame_distribution {
			extern int EXPLORATION_MANAGER;
			extern int RESOURCE_COUNTER;
		}

		/**
		 * General configuration for the game
		 */
		namespace game {
			extern int SPEED;
		}

		/**
		 * Logger settings, these variables cannot be changed.
		 */
		namespace log {
			extern const std::string OUTPUT_DIR;
			extern const std::string SETTINGS_FILE;
		}

		/**
		 * Specific variables for the squad.
		 */
		namespace squad {
			extern const std::string UNIT_COMPOSITION_DIR;
			/** How long shall the Commander wait for the first ping */
			extern double PING_WAIT_TIME_FIRST;
			/** How long shall the Commander wait for pings after the first one */
			extern double PING_WAIT_TIME_AFTER_FIRST;
			/** Decides the distance away from the center a unit has to be until a group occurs.
			 * If a unit is further away from each other than this distance from the squads
			 * center then the squads will get a temporary goal to center of the squad to
			 * regroup. 
			 * @see REGROUP_DISTANCE_END for when regrouping shall end. These two variables
			 * should not have the same value since this can cause threshold errors.
			 */
			extern double REGROUP_DISTANCE_BEGIN;
			/** Decides the distance away from the center all units has to be until a regroup
			 * is considered to be done.
			 * @see REGROUP_DISTANCE_BEGIN for the distance when regrouping shall begin.
			 */
			extern double REGROUP_DISTANCE_END;
		}

		/**
		 * Different Wait Goal sets
		 */
		namespace wait_goals {
			extern const std::string ATTACK_COORDINATION;
		}

		/**
		 * Loads settings from CONFIG_DIR\config.ini.
		 */
		void loadConfig();
	}
}