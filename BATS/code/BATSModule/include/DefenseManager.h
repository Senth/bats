#pragma once

#include <vector>
#include <map>
#include <BWAPI/TilePosition.h>
#include "TypeDefs.h"

// Forward declaration
namespace BWAPI {
	class Unit;
}
namespace BWTA {
	class Region;
	class Chokepoint;
}

// Namespace for the project
namespace bats {

// Forward declaration
class UnitManager;
class SquadManager;
class GameTime;

/**
 * Manages the defense for the team. Creates DefensiveHoldSquad and DefensivePatrolSquad
 * and checks which areas shall be defended, both around our base and around the player's
 * base.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class DefenseManager {
public:
	/**
	 * Destructor
	 */
	virtual ~DefenseManager();

	/**
	 * Returns the instance of DefensiveManager.
	 * @return instance of DefensiveManager.
	 */
	static DefenseManager* getInstance();

	/**
	 * Checks for places that needs defending and for free units to add to defending
	 * squads.
	 */
	void update();

	/**
	 * Returns all units that are free to use for another squad. This generally means all
	 * units in the DefensivePatrolSquad and DefensiveHoldSquads if we're not under attack.
	 * If we're under attack it will return no units.
	 * @return returns all free units that aren't occupied with defending. I.e. all or none.
	 */
	std::vector<BWAPI::Unit*> getAllFreeUnits();

	/**
	 * Checks whether either we or the player is under attack.
	 * @return true if we or the player is under attack.
	 */
	bool isUnderAttack() const;

	/**
	 * Prints graphical debug information.
	 * \li Defense perimeter
	 * \li Enemy offensive perimeter
	 */
	void printGraphicDebugInfo() const;

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	DefenseManager();

	/**
	 * Searches for and finds all choke points that needs defending.
	 */
	void updateDefendPositions();

	/**
	 * Checks if a region is occupied by either our structures or our allied structures.
	 * Can be overridden to only check allied or our structures.
	 * @param pRegion the region to check if it's occupied by our team
	 * @param includeOurStructures if our structures shall be included in the search,
	 * defaults to true
	 * @param includeAlliedStructures if allied structures shall be included in the search,
	 * defaults to true.
	 * @return true if the region is occupied by either any team structure.
	 */
	static bool isRegionOccupiedByOurTeam(
		BWTA::Region* pRegion,
		bool includeOurStructures = true,
		bool includeAlliedStructures = true
	);

	/**
	 * Checks if the choke point is an edge choke point. To be an edge choke point it
	 * one abut region needs to be occupied by our team and the other shall not. In addition
	 * The abut region not occupied by us needs a walkable neighbor region that is not
	 * occupied by our team. This is to prevent defending choke points that leads to an empty
	 * region that only our team can walk to�note, the enemy can, however, fly there.
	 * @note It does not matter if a choke point is occupied by enemy structures, because
	 * they are not taken into calculations.
	 * @param pChokepoint the choke point to check if it's an edge
	 * @return true if the choke point is considered to be an edge.
	 */
	static bool isChokepointEdge(BWTA::Chokepoint* pChokepoint);

	/**
	 * Searches for choke points worth defending.
	 * @return all choke points we shall defend
	 */
	static std::vector<BWTA::Chokepoint*> getDefendChokepoints();

	/**
	 * Searches for a good position to defend the choke point from. 
	 * @param pChokepoint the choke point to defend
	 * @return a good position to defend the choke point from.
	 */
	static BWAPI::TilePosition getDefendPosition(BWTA::Chokepoint* pChokepoint);

	/**
	 * Updates the move squad. I.e. adds units and a new wait position.
	 */
	void updatePatrolSquad();

	/**
	 * Checks if some defend positions are under attack. This includes enemies just being
	 * in the offensive perimeter.
	 */
	void updateUnderAttackPositions();

	/**
	 * Checks if the choke point abuts a region where we have structures. I.e. it shall be
	 * primary we who defends this area.
	 * @param pChokepoint the choke point to check
	 * @param testOur set to true to check if the choke point abuts to our region, false to check if the
	 * choke point abuts to an allied region.
	 * @return true if it's our choke point, else false and thus it's the allies's choke point.
	 * @note When returning true it does not mean it is only our or an allied choke point, it can be
	 * both our and our allied. To test this two calls needs to be made, one with testOur set to
	 * true and the other set to false.
	 */
	static bool isOurOrAlliedChokepoint(BWTA::Chokepoint* pChokepoint, bool testOur);

	struct DefendPosition {
		BWAPI::TilePosition position;
		bool underAttack; /**< The region is under attack */
		bool isOur; /**< If the choke point is abut to a region with our structures */
		bool isAllied; /**< If the choke point is abut to a region with allied structures */

		/**
		 * Default constructor
		 * @param position the position of the defended area, defaults to TilePositions::Invalid.
		 */
		DefendPosition(const BWAPI::TilePosition position = BWAPI::TilePositions::Invalid) :
		position(position), underAttack(false), isOur(false), isAllied(false) {}
	};

	UnitManager* mpUnitManager;
	SquadManager* mpSquadManager;
	GameTime* mpGameTime;
	bool mUnderAttack;

	typedef std::map<BWTA::Chokepoint*, DefendPosition> DefendMap;
	DefendMap mDefendPositions;

	int mFrameCallLast;
	static DefenseManager* mpsInstance;
};
}