#pragma once

#include "SquadDefs.h"
#include <vector>
#include <map>
#include <memory>

// Namespace for the project
namespace bats {

typedef std::map<SquadId, SquadPtr>::iterator SquadIt;
typedef std::map<SquadId, SquadPtr>::const_iterator SquadCstIt;

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
	 * Finds all squads that either is of the specified SquadType or has it as its base
	 * class.
	 * @tparam SquadType finds all squads that can be dynamically converted to this
	 * class. E.g. the type Squad would return all squads, whereas AttackSquad would return
	 * types of AttackSquad and DropSquad (since DropSquad is an AttackSquad).
	 * @return all squads that can be dynamically casted to SquadType.
	 */
	template <typename SquadType>
	std::vector<std::tr1::shared_ptr<const SquadType>> getSquads() const;

	/**
	 * /copydoc getSquads() const
	 */
	template <typename SquadType>
	std::vector<std::tr1::shared_ptr<SquadType>> getSquads();

	/**
	 * Returns the specified squad.
	 * @param squadId the squad id of the squad to return
	 * @return pointer to the squad, NULL if the squad was not found.
	 */
	SquadPtr getSquad(SquadId squadId);

	/**
	 * \copydoc getSquad()
	 */
	SquadCstPtr getSquad(SquadId squadId) const;

	/**
	 * Get the beginning of the squad iterator
	 * @return beginning of the iterator
	 */
	SquadIt begin();

	/**
	 * Get the beginning of the squad iterator as const
	 * @return beginning of the const_iterator
	 */
	SquadCstIt begin() const;

	/**
	 * Get the end of the squad iterator
	 * @return end of the squad iterator
	 */
	SquadIt end();

	/**
	 * Get the end of the squad iterator as const
	 * @return end of the squad const_iterator
	 */
	SquadCstIt end() const;

	/**
	 * Called every frame. Calls all squad's bats::Squad::computeActions() to update the squads.
	 */
	void update();

	/**
	 * Add a new squad.
	 * @param pSquad pointer to the new squad.
	 */
	void addSquad(SquadRef pSquad);

	/**
	 * Removes a squad from the squad handler.
	 * @param squadId the id of the squad to remove.
	 */
	void removeSquad(const SquadId& squadId);

	/**
	 * Prints graphical debugging information
	 */
	void printGraphicDebugInfo();

	/**
	 * Returns the frontal attack, if we have any
	 * @return frontal attack if we have any, else NULL
	 */
	AttackSquadPtr getFrontalAttack();

	/**
	 * \copydoc getFrontalAttack()
	 */
	AttackSquadCstPtr getFrontalAttack() const;

	/**
	 * Returns all distracting attacks
	 * @return all distracting attacks
	 */
	std::vector<AttackSquadPtr> getDistractingAttacks();

	/**
	 * \copydoc getDistractingAttacks()
	 */
	std::vector<AttackSquadCstPtr> getDistractingAttacks() const;

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	SquadManager();

	std::map<SquadId, SquadPtr> mSquads;
	static SquadManager* mpsInstance;
};


// Implementation of templates
template <typename SquadType>
std::vector<std::tr1::shared_ptr<const SquadType>> SquadManager::getSquads() const {
	SquadManager* pSquadManager = const_cast<SquadManager*>(this);
	const std::vector<std::tr1::shared_ptr<SquadType>>& squads = pSquadManager->getSquads<SquadType>();
	const std::vector<std::tr1::shared_ptr<const SquadType>>& squadCst = *
		reinterpret_cast<const std::vector<std::tr1::shared_ptr<const SquadType>>*>(&squads);

	return squadCst;
}

template <typename SquadType>
std::vector<std::tr1::shared_ptr<SquadType>> SquadManager::getSquads() {
	std::vector<std::tr1::shared_ptr<SquadType>> foundSquads;

	for (SquadIt squadIt = mSquads.begin(); squadIt != mSquads.end(); ++squadIt) {
		std::tr1::shared_ptr<SquadType> pSquad(
			std::tr1::dynamic_pointer_cast<SquadType>(squadIt->second)
		);

		if (NULL != pSquad) {
			foundSquads.push_back(pSquad);
		}
	}

	return foundSquads;
}
}