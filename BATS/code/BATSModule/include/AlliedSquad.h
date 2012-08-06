#pragma once

#include "PlayerSquad.h"
#include <vector>
#include <list>
#include <BWAPI/Unit.h>
#include <BWAPI/TilePosition.h>

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
class AlliedSquad : public PlayerSquad {
public:
	/**
	 * Default constructor
	 * @param frontalAttack if this squad is the main army, defaults to false.
	 */
	AlliedSquad(bool frontalAttack = false);

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
		State_Attacking, /**< Attacking, both at home or at enemy */
		State_Retreating, /**< Retreating from somewhere to home */
		State_AttackHalted /**< Outside home and still */
	};

	/**
	 * Checks whether the squad is big or not. Only one squad can be big, i.e. the main attack.
	 * @return true if this squad is the main army.
	 */
	bool isFrontalAttack() const;

	/**
	 * Change the big status of the squad. Shall only be called by AlliedArmyManager.
	 * @param frontalAttack if this squad is the main army.
	 */
	void setFrontalAttack(bool frontalAttack);

	/**
	 * Returns the current state of the AlliedSquad.
	 * @return current state of the AlliedSquad.
	 * @see AlliedSquad::States for the different states of the squad.
	 * @see isActive()
	 */
	States getState() const;

	/**
	 * Returns true if the squad is active in some way. Equal to
	 * getState() != State_idle
	 * @return true if the squad is active in some way.
	 * @see getState()
	 */
	bool isActive() const;

	/**
	 * Checks whether the squad is under attack or not
	 * @return true if the squad is under attack
	 */
	bool isUnderAttack() const;

	virtual void onConstantChanged (config::ConstantName constantName);

protected:
	virtual void updateDerived();
	virtual bool isDebugOff() const;
	virtual std::string getDebugString() const;

private:
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
	 * Updates the closest distances from allied and enemy structures
	 */
	void updateClosestDistances();

	/**
	 * Returns the current state as a string
	 * @return current state as string
	 */
	std::string getStateString() const;

	

	bool mFrontattack;
	std::list<int> mAlliedDistances;
	std::list<int> mEnemyDistances;
	States mState;
	mutable double mAttackLast;
	double mUnderAttackLast;
	mutable double mRetreatStartedTime;
	mutable double mRetreatStartTestTime;
	mutable bool mRetreatedLastCall;

	static bats::ExplorationManager* mpsExplorationManager;
};
}