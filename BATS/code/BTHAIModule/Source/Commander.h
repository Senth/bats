#ifndef __COMMANDER_H__
#define __COMMANDER_H__

#include "Squad.h"
#include "BaseAgent.h"

using namespace BWAPI;
using namespace BWTA;
using namespace std;

struct SortSquadList {
	bool operator()(Squad*& sq1, Squad*& sq2)
	{
		if (sq1->getPriority() != sq2->getPriority())
		{
			return sq1->getPriority() < sq2->getPriority();
		}
		else
		{
			if (sq1->isRequired() && !sq2->isRequired()) return true;
			else return false;
		}
	}
};

/** The Commander class is the heart of deciding high level strategies like when and where to engage the enemy.
 * It is responsible for deciding when to attack the enemy, where to attack him, and when a retreat 
 * is the best option to do.
 * All units are grouped into Squads, and the Commander directs the different Squads to attack
 * or defend positions. Each Squad are in turn responsible for handle the task it has been assigned.
 *
 * The Commander is implemented as a singleton class. Each class that needs to access Commander can request an instance,
 * and all classes shares the same Commander instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Commander {

private:
	bool chokePointFortified(TilePosition center);
	bool chokePointGuarded(TilePosition center);
	void sortSquadList();
	bool isOccupied(BWTA::Region* region);
	bool isEdgeChokepoint(Chokepoint* choke);
	double getDistToBase(Chokepoint* choke);
	TilePosition findDefensePos(Chokepoint* choke);
	double getChokepointPrio(TilePosition center);

	void checkNoSquadUnits();
	void assignUnit(BaseAgent* agent);

	int lastCallFrame;

protected:
	vector<Squad*> squads;
	int currentID;
	static Commander* instance;

	int currentState;
	static const int DEFEND = 0;
	static const int ATTACK = 1;

	Commander();

	/** Used to find where offensive attackin ground squads are, so
	 * air squads doesnt get ahead of other squads when attacking. */
	TilePosition findOffensiveSquadPosition(TilePosition closeEnemy);

public:
	/** Destructor. */
	virtual ~Commander();

	/** Returns the instance of the class. */
	static Commander* getInstance();

	/** Called each update to issue orders. */
	virtual void computeActions();

	/** Returns true if an attack against the enemy has been launched. */
	virtual bool isLaunchingAttack();

	/** Used in debug modes to show goal of squads. */
	virtual void debug_showGoal();

	/** Checks if it is time to engage the enemy. This happens when all Required squads
	 * are active. */
	virtual bool shallEngage();

	virtual void updateGoals();

	/** Called each time a unit is created. The unit is then
	 * placed in a Squad. */
	virtual void unitCreated(BaseAgent* agent);

	/** Called each time a unit is destroyed. The unit is then
	 * removed from its Squad. */
	virtual void unitDestroyed(BaseAgent* agent);

	/* Checks if the specified unittype needs to be built. */
	virtual bool needUnit(UnitType type);

	/** Returns the Squad with the specified id, or NULL if not found. */
	virtual Squad* getSquad(int id);

	/** Returns all Squads. */
	virtual vector<Squad*> getSquads();

	/** Returns the position of the closest enemy building from the start position,
	 * or TilePosition(-1,-1) if not found. */
	virtual TilePosition getClosestEnemyBuilding(TilePosition start);
	
	/** Returns the number of active offensive squads within maxRange of 
	 * the center position. */
	virtual int noOffensiveSquadsWithin(TilePosition center, int maxRange);

	/** Checks if workers needs to attack. Happens if base is under attack and no offensive units
	 * are available. */
	virtual bool checkWorkersAttack(BaseAgent* base);

	/** Tries to find a free squad to assist a building. */
	virtual void assistBuilding(BaseAgent* building);

	/** Tries to find a free squad to assist a worker that is under attack. */
	virtual void assistWorker(BaseAgent* worker);

	/** Counts the number of enemy units withing range from the start position. */
	virtual int enemyUnitsWithinRange(TilePosition start, int range);

	/** Checks if there are any removable obstacles nearby, i.e. minerals with less than 20 resources
	 * left. */
	virtual void checkRemovableObstacles();

	/** Forces an attack, even if some squads are not full. */
	virtual void forceAttack();

	/** Shows some info on the screen. */
	virtual void printInfo();

	/** Searches for a chokepoint that is unfortified, i.e. does not contain for example a Bunker or defensive
	 * turret. Returns TilePosition(-1, -1) if no position was found. */
	virtual TilePosition findUnfortifiedChokePoint();
	
	/** Searches for and returns a good chokepoint position to defend the territory. */
	virtual TilePosition findChokePoint();

	/** Checks if a position is covered by psi (Protoss only). */
	virtual bool isPowered(TilePosition pos);

	/** Checks if a position is buildable. */
	virtual bool isBuildable(TilePosition pos);

	/** Checks if there are any unfinished buildings that does not have an SCV working on them. Terran only. */
	virtual bool checkUnfinishedBuildings();

	/** Check if there are any important buildings or units to repair. Terran only. */
	virtual bool checkRepairUnits();

	/** Returns true if the unit is important to assist, false if not. All buildings and large expensive units
	 * such as siege tanks and battlecruisers are considered important, while small units such as marines and
	 * vultures are not considered important. Terran only.*/
	virtual bool isImportantUnit(Unit* unit);

	/** Assigns a worker to repair the specified agent. Terran only.*/
	virtual void repair(BaseAgent* agent);

	/** Assigns a worker to finish constructing an interrupted building. Terran only. */
	virtual void finishBuild(BaseAgent* agent);

	/** The total killScore points of all own, destroyed units (not buildings). */
	int ownDeadScore;

	/** The total killScore points of all enemy, destroyed units (not buildings). */
	int enemyDeadScore;

	/** Adds a bunker squad when a Terran Bunker has been created. */
	virtual void addBunkerSquad();
};

#endif
