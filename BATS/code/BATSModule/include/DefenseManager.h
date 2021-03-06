#pragma once

#include <vector>
#include <set>
#include <BWAPI/TilePosition.h>
#include <BWTA/Chokepoint.h>
#include "SquadDefs.h"

// Forward declaration
class UnitAgent;
class BaseAgent;

namespace BWTA {
	class Region;
}
namespace BWAPI {
	class Unit;
}

// Namespace for the project
namespace bats {

// Forward declaration
class UnitManager;
class SquadManager;
class GameTime;
class UnitCompositionFactory;
class IntentionWriter;

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
	std::vector<UnitAgent*> getFreeUnits();

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

	/**
	 * Finds a good position to retreat to.
	 * @return a position to retreat to
	 */
	BWAPI::TilePosition findRetreatPosition() const;

	/**
	 * Called when a structures is under attack
	 * @param structure the structure that is under attack
	 */
	void onStructureUnderAttack(BaseAgent* structure);

	/**
	 * Called when a worker is under attack, but only if the worker is not in a squad.
	 * @param worker the worker that is under attack
	 */
	void onWorkerUnderAttack(BaseAgent* worker);

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
	 * Checks if the choke point is an edge choke point. To be an edge choke point it
	 * one abut region needs to be occupied by our team and the other shall not. In addition
	 * The abut region not occupied by us needs a walkable neighbor region that is not
	 * occupied by our team. This is to prevent defending choke points that leads to an empty
	 * region that only our team can walk to�note, the enemy can, however, fly there.
	 * @note It does not matter if a choke point is occupied by enemy structures, because
	 * they are not taken into calculations.
	 * @param chokepoint the choke point to check if it's an edge
	 * @return true if the choke point is considered to be an edge.
	 */
	static bool isChokepointEdge(BWTA::Chokepoint* chokepoint);

	/**
	 * Searches for choke points worth defending.
	 * @return all choke points we shall defend
	 */
	static std::vector<BWTA::Chokepoint*> getDefendChokepoints();

	/**
	 * Searches for a good roaming position from where we can defend the defend position
	 * @param defendPosition the position to search from
	 * @return a roaming position for the specified defend position.
	 */
	static BWAPI::TilePosition findRoamPosition(const BWAPI::TilePosition& defendPosition);

	/**
	 * Updates the move squad. I.e. adds units and a new wait position.
	 */
	void updatePatrolSquad();

	/**
	 * Updates all the hold squads. I.e. adds, deletes and improve existing.
	 */
	void updateHoldSquads();

	/**
	 * Checks if some defend positions are under attack. This includes enemies just being
	 * in the offensive perimeter.
	 */
	void updateUnderAttackPositions();

	/**
	 * Checks if the choke point abuts a region where we have structures. I.e. it shall be
	 * primary we who defends this area.
	 * @param chokepoint the choke point to check
	 * @param testOur set to true to check if the choke point abuts to our region, false to check if the
	 * choke point abuts to an allied region.
	 * @return true if it's our choke point, else false and thus it's the allies's choke point.
	 * @note When returning true it does not mean it is only our or an allied choke point, it can be
	 * both our and our allied. To test this two calls needs to be made, one with testOur set to
	 * true and the other set to false.
	 */
	static bool isOurOrAlliedChokepoint(BWTA::Chokepoint* chokepoint, bool testOur);

	/**
	 * Checks the specified position is in our defending position list.
	 * @param position the position to check if it's in the defending list.
	 * @return true if the position is in the defending list, otherwise false.
	 */
	bool isInDefendingList(const BWAPI::TilePosition& position) const;

	/**
	 * Finds and returns the current Patrol squad
	 * @return the current patrol squad, empty (NULL) if not found.
	 */
	PatrolSquadPtr getPatrolSquad();

	/**
	 * Finds and returns a const version of Patrol squad
	 * @return current patrol squad as const, empty (NULL) if not found.
	 */
	PatrolSquadCstPtr getPatrolSquad() const;

	/**
	 * Defend unit, sends the patrol squad (if it's not defending anything else)
	 * to the specified location.
	 * @param unit the unit to defend
	 */
	void defendUnit(BWAPI::Unit* unit);

	struct DefendPosition {
		BWTA::Chokepoint const * const pChokepoint;
		const BWAPI::TilePosition position;
		bool underAttack; /**< The region is under attack */
		bool isOur; /**< If the choke point is abut to a region with our structures */
		bool isAllied; /**< If the choke point is abut to a region with allied structures */

		/**
		 * Default constructor
		 * @param pChokepoint the choke point of the defended area, defaults to NULL.
		 */
		DefendPosition(const BWTA::Chokepoint* pChokepoint = NULL) :
			pChokepoint(pChokepoint),
			position(pChokepoint != NULL ? BWAPI::TilePosition(pChokepoint->getCenter()) : BWAPI::TilePositions::Invalid),
			underAttack(false),
			isOur(false),
			isAllied(false)
		{}

		/**
		 * Assignment operator
		 * @param rhs the defend position to copy from
		 */
		DefendPosition& operator=(const DefendPosition& rhs) {
			*const_cast<const BWTA::Chokepoint**>(&pChokepoint) = rhs.pChokepoint;
			*const_cast<BWAPI::TilePosition*>(&position) = rhs.position;
			underAttack = rhs.underAttack;
			isOur = rhs.isOur;
			isAllied = rhs.isAllied;

			return *this;
		}

		/**
		 * Less than operator (for sorting in sets, maps, etc)
		 * @param rhs the other defend position to check with
		 * @return true if this choke point pointer is less than the right hand side defend position
		 */
		bool operator<(const DefendPosition& rhs) const {
			return pChokepoint < rhs.pChokepoint;
		}

		/**
		 * Checks if this defend position is equal to another defend position
		 * @param rhs the other defend position to check with
		 * @return true if it's the same defend position
		 */
		bool operator==(const DefendPosition& rhs) const {
			return pChokepoint == rhs.pChokepoint;
		}
	};

	UnitManager* mUnitManager;
	SquadManager* mSquadManager;
	GameTime* mGameTime;
	UnitCompositionFactory* mUnitCompositionFactory;
	BWAPI::Unit* mDefendUnit;
	BWAPI::Unit* mDefendAlliedUnit;
	IntentionWriter* mIntentionWriter;

	bool mUnderAttack;
	bool mAlliedUnderAttack;
	bool mDefendingAllied;
	typedef std::set<DefendPosition> DefendSet;
	DefendSet mDefendPositions;

	int mFrameCallLast;
	static DefenseManager* msInstance;
};
}