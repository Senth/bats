#pragma once

#include "AttackSquad.h"

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
	virtual void computeSquadSpecificActions();
	virtual GoalStates checkGoalState() const;

private:
	/**
	 * Loads all units into the transportations
	 */
	void loadUnits();

	bool mLoaded;
	bool mInitialized;
	double mStartTime;
};
}