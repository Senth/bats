#pragma once

#include "BTHAIModule/Source/AgentManager.h"
#include "BTHAIModule/Source/UnitAgent.h"

// Namespace for the project
namespace bats {

// Forward declarations
class SquadManager;
	
/**
 * Filters used for getting specific units
 */
enum UnitFilters {
	UnitFilter_NoFilter				= 0x0000,
	UnitFilter_NoSquad				= 0x0001,
	UnitFilter_DisbandableSquad		= 0x0002
};

/**
 * Manages all the AI's alive units.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class UnitManager : public AgentManager {
public:
	/**
	 * Destructor
	 */
	virtual ~UnitManager();

	/**
	 * Returns the instance of UnitManager.
	 * @note You must call UnitManager::getInstance() first before calling
	 * AgentManager::getInstance(), else an AgentManager will be created then
	 * you cannot create an instance of UnitManager.
	 */
	static UnitManager* getInstance();

	/**
	 * Returns all units based on the specified filter. This call only returns units
	 * of type UnitAgent, if you want all units including buildings call getAgents.
	 * instead.
	 * @param filter what filter to use. Defaults to UnitFilter_NoFilter. You can combine filters:
	 * \code
	 * std::vector<UnitAgent*> units;
	 * units = UnitManager::getUnitsByFilter(UnitFilter_NoSquad | UnitFilter_DisbandableSquad);
	 * \endcode
	 * This will return units that are either in no squads or in a squad that is disbandable.
	 * @note you cannot AND filters.
	 * @return all attacking, supporting movable units (i.e. non-building and non-worker) that match
	 * the specified filter.
	 * @todo Maybe optimize the function, it's very inefficient at the moment.
	 */
	std::vector<UnitAgent*> getUnitsByFilter(int filter = UnitFilter_NoFilter);

	/**
	 * \copydoc getUnitsByFilter()
	 */
	std::vector<const UnitAgent*> getUnitsByFilter(int filter = UnitFilter_NoFilter) const;

protected:
	/**
	 * Singleton constructor to enforce singleton usage.
	 */
	UnitManager();

	//virtual void onAgentAdded(BaseAgent* newAgent);
	virtual void onAgentDestroyed(BaseAgent* destroyedAgent);

	SquadManager* mpSquadManager;
};
}