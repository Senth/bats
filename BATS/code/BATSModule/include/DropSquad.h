#pragma once

#include "AttackSquad.h"

class TransportAgent;

// Namespace for the project
namespace bats {

/**
 * A drop squad that tries to drop at a specific location. It can either drop where
 * the player asks it to drop, or it will use AttackCoordinator to get a good drop location.
 * Drops will not try to suicide kill everything, but will retreat if the squad couldn't drop
 * at the specific location.
 * 
 * If the position for the drop is well defended by either structures or army units it will
 * try to find another position to drop using AttackCoordinator. The drop will, however,
 * timeout after a while if it hasn't unloaded the units already. Meaning it will never start
 * to unload and then just retreat when its in a favorable position. The timeout is specified
 * in the config file as 'timeout' under the [squad.drop] section.
 * 
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class DropSquad : public AttackSquad {
public:
	/**
	 * Constructs a squad with the specified unit from the unit composition. All units from
	 * the unit composition needs to be within the units parameter.
	 * @param units vector with all units for the drop, these should be the same number of
	 * units that is required by the unitComposition.
	 * @param unitComposition what units the bot shall have.
	 */
	DropSquad(const std::vector<UnitAgent*>& units, const UnitComposition& unitComposition);

	/**
	 * Destructor
	 */
	virtual ~DropSquad();

	std::string getName() const;

protected:
	virtual void updateDerived();
	virtual GoalStates checkGoalState() const;
	virtual void onUnitAdded(UnitAgent* pAddedUnit);
	virtual void onUnitRemoved(UnitAgent* pRemovedUnit);
	virtual void onGoalFailed();
	virtual void onGoalSucceeded();
	virtual void onRetreatCompleted();

	/**
	 * Uses mainly AttackSquad's createGoal to find where the goal shall be created
	 * and then creates a via path to the goal. It will always try to go on the
	 * outskirts of the map.
	 */
	virtual bool createGoal();

private:
	/**
	 * States of the Drop
	 */
	enum States {
		State_Attack,	/**< When the squad attacks something */
		State_Load,		/**< When the squad loads units into transports */
		State_Transport	/**< When the squad transports all ground units */
	};

	/**
	 * Loads all units into the transportations
	 */
	void loadUnits();

	/**
	 * Unloads all units from the transportations
	 */
	void unloadUnits();

	/**
	 * Checks if all enemy units within sight, that can attack, are faster than our transport
	 * @return true if any enemy unit can travel faster than our transportation
	 */
	bool isEnemyFasterThanTransport() const;

	/**
	 * Check if the specified units are faster than us
	 * @param enemyUnits units to check if they are faster than our transportation
	 * @return true if any enemy unit can travel faster than our transportation
	 */
	bool isEnemyFasterThanTransport(const std::vector<BWAPI::Unit*> enemyUnits) const;

	/**
	 * Check if all transports are in the same region as the goal.
	 * @return true if all transports are in the same region as the goal
	 */
	bool isTransportsInGoalRegion() const;

	/**
	 * Sets the new state of the drop. This handles some extra functionality like
	 * setting transportation to load/unload
	 * @param newState the new state
	 */
	void setState(States newState);

	/**
	 * Checks whether the transports are done loading all units
	 * @return true if all transports are done loading units
	 */
	bool isTransportsDoneLoading() const;

	/**
	 * Creates and replaces the via path. It will try to go along the map edges
	 * until it comes to the border closest to the position, then it will continue to
	 * go either to the retreat position or the goal.
	 */
	void createViaPath();

	/**
	 * Checks if the drop attack has timed out
	 * @return true if the drop attack has timed out
	 */
	bool hasAttackTimedOut() const;

	States mState;
	bool mInitialized;
	double mStartTime;
	double mLoadStart;
	bool mFailed;
	std::set<TransportAgent*> mTransports;
};
}