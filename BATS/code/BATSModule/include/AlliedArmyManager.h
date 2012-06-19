#pragma once

#include "IdTypes.h"
#include "Config.h"
#include <memory>
#include <vector>
#include <map>
#include <BWAPI/Unit.h>
#include <BWAPI/Position.h>

// Namespace for the project
namespace bats {

// Forward declarations
class AlliedSquad;

/**
 * Manages allies virtual squads. It created, splits, and merges units that are within the squads
 * Variables include_distance and exclude_distance under [classification.squad] 
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class AlliedArmyManager : public config::OnConstantChangedListener {
public:
	/**
	 * Destructor
	 */
	virtual ~AlliedArmyManager();

	/**
	 * Returns the instance of AlliedArmyManager.
	 * @return instance of AlliedArmyManager.
	 */
	static AlliedArmyManager* getInstance();

	/**
	 * Updates the squads. Splits squads, moves units from one squad to another, and disbands
	 * squads if no units is within the squad.
	 */
	void update();

	/**
	 * Called when a constant has been updated (that this class listens to).
	 * Currently it only listens these variables:
	 * \code
	 * [classification.squad]
	 * grid_square_distance
	 * \endcode
	 */
	void onConstantChanged(config::ConstantName constantName);

private:
	/**
	 * Singleton constructor to enforce singleton usage.
	 */
	AlliedArmyManager();

	/**
	 * Returns the grid position of the unit
	 */
	BWAPI::Position getGridPosition(BWAPI::Unit* pUnit) const;

	/**
	 * Recalculate the lookup table
	 */
	void recalculateLookupTable();

	std::vector<std::tr1::shared_ptr<AlliedSquad>> mSquads;
	std::map<BWAPI::Unit*, AlliedSquadId> mUnitSquad;

	// Look-up table for where the unit is located.
	std::vector<std::vector<BWAPI::Position>> mLookupTableGridPosition;

	static AlliedArmyManager* mpsInstance;
};
}