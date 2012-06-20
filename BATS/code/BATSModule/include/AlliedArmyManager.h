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
 * @todo split squads by players, as of now the players units are grouped into the same squad
 * if they are close to each other even if they belong to two different allies.
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
	 * Rearranges the squads so that all limitations are met. Meaning it will split squads,
	 * move units from one squad to another squad if squads are within include_distance or
	 * outside exclude_distance
	 */
	void rearrangeSquads();

	/**
	 * Called when a constant has been updated (that this class listens to).
	 * Currently it only listens these variables:
	 * \code
	 * [classification.squad]
	 * grid_square_distance
	 * \endcode
	 */
	void onConstantChanged(config::ConstantName constantName);

	/**
	 * Add an allied unit to the manager. Do this when a unit is created.
	 * @param pUnit the new unit 
	 */
	void addUnit(BWAPI::Unit* pUnit);

	/**
	 * Remove an allied unit from the manager. Do this when a unit is killed.
	 * @param pUnit the killed unit.
	 */
	void removeUnit(BWAPI::Unit* pUnit);

private:
	/**
	 * Singleton constructor to enforce singleton usage.
	 */
	AlliedArmyManager();

	/**
	 * Returns the grid position of the unit
	 * @param pUnit unit that check the corresponding grid placement of
	 * @pre pUnit is not NULL
	 * @return grid position of the unit.
	 */
	BWAPI::Position getGridPosition(BWAPI::Unit* pUnit) const;

	/**
	 * Recalculate the lookup table. Shall be called whenever grid_square_distance is
	 * changed.
	 */
	void recalculateLookupTable();

	/**
	 * Add a newly created AlliedSquad to the squad list.
	 * @param pSquad pointer to the newly created squad
	 */
	void addSquad(AlliedSquad* pSquad);

	/**
	 * Removes an AlliedSquad from the squad list and deletes it
	 * @param squadId the squad id of the squad
	 */
	void removeSquad(AlliedSquadId squadId);

	std::vector<std::tr1::shared_ptr<AlliedSquad>> mSquads;
	std::map<BWAPI::Unit*, AlliedSquadId> mUnitSquad;

	// Look-up table for where the unit is located.
	std::vector<std::vector<BWAPI::Position>> mLookupTableGridPosition;

	static AlliedArmyManager* mpsInstance;
};
}