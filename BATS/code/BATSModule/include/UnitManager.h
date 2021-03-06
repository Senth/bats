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
	/** All units including workers. Same as UnitFilter_UnitsAll | UnitFilter_WorkersAll */
	UnitFilter_NoFilter				= 0x0000,
	UnitFilter_HasNoSquad			= 0x0001,	/**< Non-workers that aren't in a squad */
	UnitFilter_InDisbandableSquad	= 0x0002,	/**< Non-workers that are in a disbandable squad */
	/** Non-workers that are either not in a squad, or all defensive squad units when not under attack */
	UnitFilter_Free					= 0x0004,
	UnitFilter_UnitsAll				= 0x0008,	/**< All non-workers */
	/** All transportation units, does not work alone but in combination with other unit
	 * filters. */
	UnitFilter_Transportation		= 0x0010,	
	UnitFilter_WorkersAll			= 0x0100,	/**< All workers */
	UnitFilter_WorkersFree			= 0x0200	/**< Workers in no squads */
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
	 * instead. Does not support limiting the filter to all filter at the moment, it will use
	 * OR on all filters.
	 * @param filter what type of units you want to get. Defaults to UnitFilter_NoFilter
	 * which will return all units including workers.
	 * @return all attacking, supporting movable units (i.e. non-building and non-worker) that match
	 * the specified filter.
	 * @see UnitFilters for all available filters.
	 * @todo Maybe optimize the function, it's very inefficient at the moment.
	 * 
	 * @section Examples
	 * Returns all units (excluding workers) that is either in no squad or in a squad
	 * that is disbandable.
	 * \code
	 * UnitManager* unitMananger = UnitManager::getInstance();
	 * std::vector<UnitAgent*> units;
	 * units = unitManager->getUnitsByFilter(UnitFilter_HasNoSquad | UnitFilter_InDisbandableSquad);
	 * \endcode
	 * 
	 * Returns all units except workers
	 * \code
	 * // ...
	 * units = UnitManager->getUnitsByFilter(UnitFilter_UnitsAll);
	 * \endcode
	 * 
	 * Returns all worker units
	 * \code
	 * // ...
	 * units = unitManager->getUnitsByFilter(UnitFilter_WorkersAll);
	 * \endcode
	 * 
	 * Returns all units, including workers, that aren't in a squad.
	 * \code
	 * // ...
	 * units = unitManager->getUnitsByFilter(UnitFilter_HasNoSquad | UnitFilter_WorkersNoSquad);
	 * \endcode
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

	virtual void onAgentDestroyed(BaseAgent* destroyedAgent);

	SquadManager* mpSquadManager;
};
}