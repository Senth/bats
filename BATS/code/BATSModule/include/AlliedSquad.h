#pragma once

#include "TypeDefs.h"
#include "Config.h"
#include <vector>
#include <list>
#include <BWAPI/Unit.h>
#include <BWAPI/TilePosition.h>
#include "Utilities/KeyHandler.h"
#include "Utilities/KeyType.h"

// Namespace for the project
namespace bats {

// forward declarations
class ExplorationManager;
class GameTime;

/**
 * Squad for allied units, such as the player playing with the bot.
 * This is a collection of units close to each other and where they are going.
 * The squad keeps track of the direction and how far it has traveled the last X seconds,
 * X is defined in the config file as measure_time under [classification.squad].
 * The squad itself can defer if it's moving to attack or is retreating and if it's a big
 * squad.
 * 
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class AlliedSquad : public config::OnConstantChangedListener {
public:
	/**
	 * Default constructor
	 * @param big if this squad is the main army, defaults to false.
	 */
	AlliedSquad(bool big = false);

	/**
	 * Destructor
	 */
	virtual ~AlliedSquad();

	/**
	 * Allied squad states
	 */
	enum States {
		State_Idle,	/**< Home in base */
		State_MovingToAttack, /**< Outside home and moving */
		State_Attacking, /**< Attacking, both @home or @enemy */
		State_Retreating, /**< Retreating from somewhere to home */
		State_AttackHalted /**< Outside home and still */
	};

	/**
	 * Checks whether the squad is big or not. Only one squad can be big, i.e. the main attack.
	 * @return true if this squad is the main army.
	 */
	bool isBig() const;

	/**
	 * Change the big status of the squad. Shall only be called by PlayerArmy.
	 * @param big if this squad is the main army.
	 */
	void setBig(bool big);

	/**
	 * Returns all units in the squad.
	 * @return all units in the squad.
	 */
	const std::vector<BWAPI::Unit*> getUnits() const;

	/**
	 * Returns the amount of supply the squad occupies.
	 * @note this is the double amount of supply since BWAPI uses this supply count
	 * because Zerglings take up 0.5 supply.
	 * @return amount of supply the squad occupies.
	 */
	int getSupplyCount() const;

	/**
	 * Returns the number of units in the squad
	 * @return number of units in the squad
	 */
	size_t getUnitCount() const;

	/**
	 * Returns true if the squad is empty.
	 * @see getNrOfUnits()
	 * @return true if the squad is empty.
	 */
	bool isEmpty() const;

	/**
	 * Adds a unit to the squad.
	 * @param pUnit the unit that shall be added to the squad.
	 */
	void addUnit(BWAPI::Unit* pUnit);

	/**
	 * Removes a unit from the squad.
	 * @param pUnit the unit that shall be removed.
	 */
	void removeUnit(BWAPI::Unit* pUnit);

	/**
	 * Returns the id of the squad.
	 * @return id of the squad
	 */
	AlliedSquadId getId() const;

	/**
	 * Called when a constant has been updated (that this class listens to).
	 * Currently it only listens these variables:
	 * \code
	 * [classification.squad]
	 * measure_time
	 * \endcode
	 */
	void onConstantChanged (config::ConstantName constantName);

	/**
	 * Returns the current state of the AlliedSquad.
	 * @return current state of the AlliedSquad.
	 * @see AlliedSquad::States for the different states of the squad.
	 */
	States getState() const;

	/**
	 * Updates the center of the squad for isMoving() and isRetreaning() calculations
	 */
	void update();

	/**
	 * Prints graphical debug information, id of squad, number of units and
	 * number of supply in the center of the squad.
	 */
	void printGraphicDebugInfo();

	/**
	 * Returns the maximum amount of AlliedSquads that are allowed to be created.
	 * @return maximum amount of AlliedSquads that are allowed to be created.
	 */
	static int getMaxKeys();

	/**
	 * Returns the current center position of the squad.
	 * @return current center position of the squad.
	 */
	const BWAPI::TilePosition& getCenter() const;

	/**
	 * Returns the target position of the squad, e.g. where the units are moving to
	 * @return squads target position, TilePositions::Invalid if no target was found.
	 */
	BWAPI::TilePosition getTargetPosition() const;

private:
	/**
	 * Returns the direction of the squad, the direction is measured by the current center
	 * position and center position for measure_time seconds ago.
	 * @note the direction is not normalized.
	 * @return direction of the squad. TilePositions::Invalid if the squad doesn't have
	 * enough readings.
	 */
	BWAPI::TilePosition getDirection() const;

	/**
	 * Returns the squared distance traveled since measure_time seconds ago.
	 * @return squared distance traveled since measure_time seconds ago.
	 */
	double getDistanceTraveledSquared() const;

	/**
	 * Calculate whether the squad is moving to attack or not.
	 * @return true when
	 * \li Moving towards enemy base or away from our bases, delta MEASURE_TIME
	 * \li Moved MOVED_TILES_MIN, delta MEASURE_TIME
	 * \li Squad center is at least AWAY_DISTANCE from our structures
	 */
	bool isMovingToAttack() const;

	/**
	 * Checks if the retreat has timed out
	 * @return true if the retreat has passed RETREAT_TIMEOUT
	 */
	bool hasRetreatTimedout() const;

	/**
	 * Calculates when the squad is retreating. This takes into account if the squad is 
	 * attacking or under attack. If the squad is attacking or under attack it is more
	 * generous setting the squad as retreating as it will return true as fast as 
	 * isRetreatingFrame() returs true. If on the other hand the squad is safe and hasn't
	 * been attacking or attacked for ATTACK_TIMEOUT seconds isRetreatingFrame() needs
	 * to return true continuously for at least RETREAT_TIME_WHEN_SAFE seconds.
	 * @return true when the squad is treated as retreating
	 * @see isRetreatingFrame()
	 */
	bool isRetreating() const;

	/**
	 * Calculate whether the squad is retreating for the current frame. This doesn't mean
	 * that the squad shall be treated as retreating.
	 * @return true when
	 * <ul>
	 * <li>Moving away from closest enemy structure, delta measure_time</li>
	 * <li>If no enemy structure is available: moving towards our bases</li>
	 * <li>Squad center is at least away_distance from our structures</li>
	 * <li>Moved moved_tiles_min, delta measure_time</li>
	 * </ul>	
	 * @see isRetreating() that acts as a wrapper for this function.
	 */
	bool isRetreatingFrame() const;

	/**
	 * Checks whether the squad is attacking something. This will always return
	 * true for ATTACK_TIMEOUT seconds after it actually attacked the last time.
	 * It still checks if the squad is attacking even when the attack hasn't timed out,
	 * this will update the time of the timeout.
	 * @return true when the squad is treated as attacking something, or has done so within
	 * ATTACK_TIMEOUT seconds.
	 */
	bool isAttacking() const;

	/**
	 * Checks whether the squad is attacking something this specific frame,
	 * Could be attacking something either at home or somewhere else.
	 * @return true if the squad is attacking something
	 * @see isAttacking() that acts as a wrapper for this function.
	 */
	bool isAttackingFrame() const;

	/**
	 * Checks whether the squad has stopped to move while moving to attack outside home
	 * @return true if the squad is outside home and 'idle'
	 */
	bool hasAttackHalted() const;

	/**
	 * Updates the center position
	 */
	void updateCenter();

	/**
	 * Updates the closest distances from allied and enemy structures
	 */
	void updateClosestDistances();

	/**
	 * Returns the current state as a string
	 * @return current state as string
	 */
	std::string getStateString() const;

	bool mBig;
	std::vector<BWAPI::Unit*> mUnits;
	std::list<BWAPI::TilePosition> mCenter;
	std::list<int> mAlliedDistances;
	std::list<int> mEnemyDistances;
	AlliedSquadId mId;
	States mState;
	double mUpdateLast;
	mutable double mAttackLast;
	double mUnderAttackLast;
	mutable double mRetreatStartedTime;
	mutable double mRetreatStartTestTime;
	mutable bool mRetreatedLastCall;

	static utilities::KeyHandler<_AlliedSquadType>* mpsKeyHandler;
	static int mcsInstances;
	static bats::ExplorationManager* mpsExplorationManager;
	static GameTime* mpsGameTime;
};
}