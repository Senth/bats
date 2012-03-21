#pragma once

#include "IdTypes.h"
#include <vector>
#include <map>

// Namespace for the project
namespace bats {

// Forward declarations
class Squad;

/**
 * Handles all squads.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class SquadManager {
public:
	/**
	 * Destructor
	 */
	virtual ~SquadManager();

	/**
	 * Returns the instance of this class
	 */
	static SquadManager* getInstance();

	/**
	 * Returns the specified squad.
	 * @param squadId the squad id of the squad to return
	 * @return pointer to the squad, NULL if the squad was not found.
	 */
	Squad* getSquad(const SquadId& squadId);

	/**
	 * \copydoc getSquad()
	 */
	Squad const * const getSquad(const SquadId& squadId) const;

	/**
	 * Get the beginning of the squad iterator
	 * @return beginning of the iterator
	 */
	std::map<SquadId, Squad*>::iterator begin();

	/**
	 * Get the beginning of the squad iterator as const
	 * @return beginning of the const_iterator
	 */
	std::map<SquadId, Squad*>::const_iterator begin() const;

	/**
	 * Get the end of the squad iterator
	 * @return end of the squad iterator
	 */
	std::map<SquadId, Squad*>::iterator end();

	/**
	 * Get the end of the squad iterator as const
	 * @return end of the squad const_iterator
	 */
	std::map<SquadId, Squad*>::const_iterator end() const;

	/**
	 * Called every frame. Calls all squad's bats::Squad::computeActions() to update the squads.
	 */
	void computeActions();

	/**
	 * Add a new squad.
	 * @param pSquad pointer to the new squad.
	 */
	void addSquad(Squad* pSquad);

	/**
	 * Removes a squad from the squad handler.
	 * @param squadId the id of the squad to remove.
	 */
	void removeSquad(const SquadId& squadId);

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	SquadManager();

	std::map<SquadId, Squad*> mSquads;
	static SquadManager* mpsInstance;
};
}