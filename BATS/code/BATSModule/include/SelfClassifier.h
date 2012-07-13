#pragma once

#include <vector>
#include <BWAPI/UnitType.h>

// forward declarations
class AgentManager;

// Namespace for the project
namespace bats {

// Formard declarations
class BuildPlanner;


/**
 * Checks various states for the bot itself. These include
 * \li If the bot is currently expanding
 * \li upgrades that will finish soon
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class SelfClassifier {
public:
	/**
	 * Destructor
	 */
	virtual ~SelfClassifier();

	/**
	 * Returns the instance of SelfClassifier.
	 * @return instance of SelfClassifier.
	 */
	static SelfClassifier* getInstance();

	/**
	 * Checks if the bot is currently expanding, includes going to expand.
	 * @return true if the bot is currently expanding or going to expand.
	 */
	bool isExpanding() const;

	/**
	 * Checks if an upgrade, that will have an effect of any of the specified units,
	 * soon is done. Soon is specified in config file as config::classifier::UPGRADE_SOON_DONE
	 * @param affectedUnitTypes all unit types that are or will be used to attack/defend.
	 * @return true if an upgrade, that will have an effect of any of the specified units,
	 * soon is done.
	 */
	bool isUpgradeSoonDone(const std::vector<BWAPI::UnitType>& affectedUnitTypes) const;

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	SelfClassifier();

	AgentManager* mpAgentManager;
	BuildPlanner* mpBuildPlanner;

	static SelfClassifier* mpsInstance;
};
}