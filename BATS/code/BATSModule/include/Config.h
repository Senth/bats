#pragma once

#include <string>

namespace bats {
	/**
	 * Configuration variables for BATS. Most of these can be configured in
	 * the config files. config_default.ini and config_override.ini
	 */
	namespace config {
		typedef void const * const ConstantName;
		/**
		 * Class for listening to constants that have been updated.
		 * Classes shall be derived from this
		 */
		class OnConstantChangedListener {
		public:
			virtual void onConstantChanged(ConstantName constantName) = 0;
		};

/**
 * Converts a constant to a "global" ConstantName
 * @param constant the constant to convert
 * @return name of the type ConstantName
 */
#define TO_CONSTANT_NAME(constant) const_cast<bats::config::ConstantName>(reinterpret_cast<void*>(&constant))

		/**
		 * Add a listener for the specified variable
		 * @param constantName name of the constant. Use macro TO_CONSTANT_NAME().
		 * @param pListener the instance that listens to the change
		 */
		void addOnConstantChangedListener(
			ConstantName constantName,
			OnConstantChangedListener* pListener
		);

		/**
		 * Removes an already existing listener
		 * @param constantName name of the constant. Use macro TO_CONSTANT_NAME().
		 * @param pListener the instance that listens to the change
		 */
		void removeOnConstantChangedListener(
			ConstantName constantName,
			OnConstantChangedListener* pListener
		);

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
		 * Classification rules for player and enemies
		 */
		namespace classification {
			/**
			 * Rules for how to group squads and what they are doing
			 */
			namespace squad {
				/** Time to measure distance traveled and the direction of the squad */
				extern size_t MEASURE_TIME;
				/** How long time must have passed since the squad has attacked or been attacked
				 * before it is actually treated as non-attacking or non-attacked. In
				 * seconds.
				 * @note the squad can still be treated as retreating although it is
				 * attacking or being attacked */
				extern double ATTACK_TIMEOUT;
				/** How long time must have passed since the squad has started it's retreating
				 * before it is actually treated as non-retreating. I.e. no other state can
				 * override retreating until at least this amount of time has passed. */
				extern double RETREAT_TIMEOUT;
				/** How long time must have passed until the squad is treated as retreating
				 * when it is safe. Used when the squad isn't attacking or is under attack.
				 * When the squad is attacking or under attack, it will use MOVED_TILES_MIN
				 * to see if the squad is retreating or not */
				extern double RETREAT_TIME_WHEN_SAFE;
				/** Minimum distance a squad shall move until it is treated as attacking or
				 * retreating */
				extern int MOVED_TILES_MIN;
				/** Cannot be set through the config file, will set automatically,
				 * used for faster calculation. Does not send an event.
				 * @see MOVED_TILES_MIN. */
				extern int MOVED_TILES_MIN_SQUARED;
				/** How far away from our home (allied structures) the squad has to be until
				 * it can be treated as either attacking or retreating. */
				extern int AWAY_DISTANCE;
				/** Cannot be set through the config file, will set automatically, used for
				 * faster calculation. Does not send an event.
				 * @see AWAY_DISTANCE */
				extern int AWAY_DISTANCE_SQUARED;
				/** Distance a unit has to be to another unit in the squad until
				 * it is treated as included in the squad. The distance is in TilePositions. */
				extern int INCLUDE_DISTANCE;
				/** Cannot be set through the config file, will set automatically,
				 * used for faster calculation. Does not send an event.
				 * @see INCLUDE_DISTANCE */
				extern int INCLUDE_DISTANCE_SQUARED;
				/** If a unit is in a squad and the distance to the closest squad unit is
				 * larger than this, it is no longer treated as part of the squad, i.e. 
				 * excluded from the squad. The distance is in TilePositions.
				 * @pre Only set this variable to an even number. because of the
				 * GRID_SQUARE_DISTANCE calculation. */
				extern int EXCLUDE_DISTANCE;
				/** Cannot be set through the config file, will set automatically,
				 * used for faster calculation. Does not send an event.
				 * @see EXCLUDE_DISTANCE */
				extern int EXCLUDE_DISTANCE_SQUARED;
				/** Cannot be set through the config file, will set automatically when
				 * exclude_distance is set. The distance each square in the grid has.
				 * This is used for faster calculation which units are close etc. Current
				 * equation is exclude_distance/2. This means that units in squares to the
				 * left/right/up/down is within the exclude_distance. If a unit
				 * is more than 2 squares away it is absolutely further away than
				 * exclude_distance. */
				extern int GRID_SQUARE_DISTANCE;
			}
		}

		/**
		 * Debugging configuration
		 */
		namespace debug {
			/** Default debug value in debugging mode */
			extern int GRAPHICS_VERBOSITY_IN_DEBUG;
			/** Default debug value in release mode */
			extern int GRAPHICS_VERBOSITY_IN_RELEASE;
			/** Current debug value, this get initially set by GRAPHICS_VERBOSITY_IN_X
			 * where X is the current mode running. Cannot be set in the config file. */
			extern int GRAPHICS_VERBOSITY;
			/** Graphical spacing between columns when printing information */
			extern int GRAPHICS_COLUMN_WIDTH;

			/**
			 * Enumerations for the graphics debugging levels
			 */
			enum GraphicsVerbosities {
				GraphicsVerbosity_Off = 0,
				GraphicsVerbosity_Low,
				GraphicsVerbosity_Medium,
				GraphicsVerbosity_High
			};

			/**
			 * Enabled classes, if these are enable they will show
			 * debugging information, provided that GRAPHICS_VERBOSITY is set
			 * to something other than Off.
			 * @note Some classes only print out debugging information for medium or high.
			 */
			namespace classes {
				extern bool ALLIED_SQUAD;
				extern bool ALLIED_ARMY_MANAGER;
				extern bool AGENT_UNIT;
				extern bool AGENT_STRUCTURE;
				extern bool AGENT_WORKER;
			}
		}

		/**
		 * Build order configuration variables
		 */
		namespace build_order {
			/** Directory of the build orders */
			extern const std::string DIR;
		}

		/**
		 * How often classes shall be called.
		 */
		namespace frame_distribution {
			extern int EXPLORATION_MANAGER;
			extern int RESOURCE_COUNTER;
			extern int ALLIED_ARMY_REARRANGE_SQUADS;
		}

		/**
		 * General configuration for the game
		 */
		namespace game {
			/** The default speed of the game */
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
			/** Decides the distance away from the center a unit has to be until a regroup occurs.
			 * If a unit is further away than this it will get a regroup goal to the center of
			 * the squad to regroup.
			 * @see REGROUP_DISTANCE_END for when regrouping shall end. These two variables
			 * should not have the same value since this can cause threshold errors.
			 */
			extern int REGROUP_DISTANCE_BEGIN;
			/** Cannot be set through the config file, will set automatically,
			 * used for faster calculation. Does not send an event.
			 * @see REGROUP_DISTANCE_BEGIN */
			extern int REGROUP_DISTANCE_BEGIN_SQUARED;
			/** Decides the distance away from the center all units has to be until a regroup
			 * is considered to be done.
			 * @see REGROUP_DISTANCE_BEGIN for the distance when regrouping shall begin.
			 */
			extern int REGROUP_DISTANCE_END;
			/** Cannot be set through the config file, will set automatically,
			 * used for faster calculation. Does not send an event.
			 * @see REGROUP_DISTANCE_END */
			extern int REGROUP_DISTANCE_END_SQUARED;
			/** Time before trying with a new regroup position when a unit is still */
			extern double REGROUP_NEW_POSITION_TIME;
			/** How long time before recalculating the distance to the unit furthest away
			 * from the squad. This is the minimum time and the distance will not be
			 * recalculated until the function is called again. */
			extern double CALC_FURTHEST_AWAY_TIME;
			/** The default range when calculating distance between the center of the squad
			 * and a position to test whether the squad is close to this position or not. */
			extern int CLOSE_DISTANCE;
			/** How much the sight distance shall be multiplied with, this is used to check
			 * if enemy units are within sight distance. The sight circle center position is the
			 * center of the squad and the radius is unit with longest sight distance multiplied
			 * with this variable. */
			extern double SIGHT_DISTANCE_MULTIPLIER;

			/**
			 * Variables for AttackSquad
			 */
			namespace attack {
				/** The distance from the goal that the waiting position will be set. If
				 * the squad is a ground squad it will use ground distance and flying units
				 * will use direct distance. */
				extern int WAITING_POSITION_DISTANCE_FROM_GOAL;
				/** The distance from the goal position to check if all buildings have
				 * been destroyed to complete the goal. */
				extern int STRUCTURES_DESTROYED_GOAL_DISTANCE;
				/** Maximum distance to an allied squad when searching for another
				 * squad once either the current following squad died or merged. */
				extern int FIND_ALLIED_SQUAD_DISTANCE;
				/** Distance away from allied squad when regrouping with allied squad begins. */
				extern int ALLIED_REGROUP_BEGIN;
				/** Cannot be set through the config file, will set automatically,
				 * used for faster calculation. Does not send an event.
				 * @see ALLIED_REGROUP_BEGIN */
				extern int ALLIED_REGROUP_BEGIN_SQUARED;
				/** Distance away from allied squad when regrouping is finished. */
				extern int ALLIED_REGROUP_END;
				/** Cannot be set through the config file, will set automatically,
				 * used for faster calculation. Does not send an event.
				 * ALLIED_REGOURP_END_SQUARED */
				extern int ALLIED_REGROUP_END_SQUARED;
			}

			/**
			 * Variables for DropSquad
			 */
			namespace drop {
				/** The time in seconds before a drop is treated as timed out. */
				extern double ATTACK_TIMEOUT;
				/** Time before a load is treated as timed out. It will then try to 
				 * load all units again */
				extern double LOAD_TIMEOUT;
			}
		}

		/**
		 * Different Wait Goal sets
		 */
		namespace wait_goals {
			extern const std::string ATTACK_COORDINATION;
		}

		/**
		 * Loads settings from CONFIG_DIR\config_default.ini and CONFIG_DIR\config_override.ini, in
		 * that order.
		 */
		void loadConfig();
	}
}