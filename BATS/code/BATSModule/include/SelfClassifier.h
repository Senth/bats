#pragma once

#include <vector>
#include <BWAPI/UnitType.h>

// forward declarations
class AgentManager;
class CommandCenterAgent;
class UnitAgent;

// Namespace for the project
namespace bats {

// Forward declarations
class BuildPlanner;
class SquadManager;
class GameTime;
class DefenseManager;
class UnitManager;
class UnitCompositionFactory;

/**
 * Checks various states the bot has. This is essentially a container of functionality
 * found (and could be derived) from many other classes.
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
	 * soon is done. Soon is specified in the config file as
	 * config::classifier::UPGRADE_SOON_DONE
	 * @param affectedUnits all units too check if any upgrade will have any effect on them
	 * @return true if an upgrade, that will have an effect of any of the specified units
	 * soon is done.
	 */
	bool isUpgradeSoonDone(const std::vector<UnitAgent*>& affectedUnits) const;

	/**
	 * Checks if all expansions are saturated with workers.
	 * @return true the worker count is enough to saturate all expansions
	 */
	bool areExpansionsSaturated() const;

	/**
	 * Checks if an expansion is running low on minerals.
	 * @note Does NOT return true for an expansion that has no minerals.
	 * @return true if an expansion is low on minerals, but above 0.
	 */
	bool isAnExpansionLowOnMinerals() const;

	/**
	 * Returns the number of active expansions. Active in this meaning is expansions that
	 * have minerals above the low limit (used for isAnExpansionLowOnMinerals()).
	 * @return number of active expansions
	 */
	int getActiveExpansionCount() const;

	/**
	 * Checks if we're currently attacking something, i.e. we have an attack squad
	 * @return true if we have an attack squad.
	 */
	bool isAttacking() const;

	/**
	 * Checks if we have an active frontal attack
	 * @return true if we have a frontal attack
	 */
	bool hasFrontalAttack() const;

	/**
	 * Checks if we have an active drop
	 * @return true if we have an active drop
	 */
	bool hasDrop() const;

	/**
	 * Checks if we have the ability to create a drop, uses free units
	 * @return true if we can create a drop
	 */
	bool canDrop() const;

	/**
	 * Checks if we have enough units to do a frontal attack, uses free units
	 * @return true if we can do a frontal attack
	 */
	bool canFrontalAttack() const;

	/**
	 * Checks if we have enough units to do a frontal attack with the specified units
	 * @param units units to do the attack with
	 * @return true if we have enough units to do a frontal attack
	 */
	bool canFrontalAttack(const std::vector<const UnitAgent*>& units) const;

	/**
	 * \copydoc canFrontalAttack(const std::vector<const UnitAgent*>&)
	 */
	bool canFrontalAttack(const std::vector<UnitAgent*>& units) const;

	/**
	 * Checks if we're currently under attack
	 * @return true if we're under attack
	 */
	bool isUnderAttack() const;

	/**
	 * Checks how many seconds has elapsed since the last expansion was started. If an
	 * expansion gets killed it will not count this one, meaning if the last built expansion
	 * was killed it will return the time from the penultimate expansion built instead.
	 * @return seconds since the last alive expansion was started.
	 */
	double getLastExpansionStartTime() const;

	/**
	 * Checks if we have an active scout squad.
	 * @return true if we have a scout, false otherwise.
	 */
	bool isScouting() const;

	/**
	 * Checks if we're high on minerals, configured in the config file, under
	 * [classification] high_on_minerals
	 * @return true if we're low on minerals
	 * @see isHighOnGas()
	 * @see isHighOnResources()
	 */
	bool isHighOnMinerals() const;

	/**
	 * Checks if we're high on gas, configured in the config file, under 
	 * [classification] high_on_gas
	 * @return true if we're low on gas
	 * @see isHighOnMinerals()
	 * @see isHighOnResources()
	 */
	bool isHighOnGas() const;

	/**
	 * Checks if we're high on resources, this function is the same as calling
	 * (isHighOnMinerals() && isHighOnGas()).
	 * @return true if we're both high on minerals and gas.
	 * @see isHighOnMinerals()
	 * @see isHighOnGas()
	 */
	bool isHighOnResources() const;

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	SelfClassifier();

	/**
	 * Checks if the specified expansion is low on minerals. The low value is configured
	 * in the config file. Section: classification.expansion, Name: expansion_minerals_low
	 * (EXPANSION_MINERALS_LOW)
	 * @param mainStructure the expansion to check if it's low on minerals
	 * @return true if the expansion is low on minerals, otherwise false.
	 */
	bool hasExpansionLowOrNoMinerals(const CommandCenterAgent* mainStructure) const;

	const AgentManager* mAgentManager;
	const BuildPlanner* mBuildPlanner;
	const SquadManager* mSquadManager;
	const DefenseManager* mDefenseManager;
	const UnitManager* mUnitManager;
	const UnitCompositionFactory* mUnitCompositionFactory;
	const GameTime* mGameTime;

	static SelfClassifier* msInstance;
};
}