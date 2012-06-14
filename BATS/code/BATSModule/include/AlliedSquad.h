#pragma once

#include "IdTypes.h"
#include "Config.h"
#include <vector>
#include <list>
#include <BWAPI/Unit.h>
#include <BWAPI/TilePosition.h>
#include "Utilities/KeyHandler.h"
#include "Utilities/KeyType.h"

// Namespace for the project
namespace bats {

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
	void onConstantChanged (
		const std::string& section,
		const std::string& subsection,
		const std::string& variable
	);

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
	 * \li Moving towards enemy base, delta measure_time
	 * \li Moved moved_tiles_min, delta measure_time
	 * \li Squad center is at least attack_percent_away_min percentage away from
	 * our structures. Uses closest buildings from allied to enemy.
	 */
	bool isMovingToAttack() const;

	/**
	 * Calculate whether the squad is retreating, almost same as isMovingToAttack().
	 * @return true when
	 * \li Moving away from closest enemy structure, delta measure_time
	 * \li Moved moved_tiles_min, delta measure_time
	 * \li Squad center is at least retreat_percent_away_min percentage away from our structures.
	 * Uses closest buildings from allied to enemy.
	 */
	bool isRetreating() const;

	/**
	 * Checks whether the squad is attacking something, either at home or somewhere else.
	 * @return true if the squad is attacking something
	 */
	bool isAttacking() const;

	/**
	 * Checks whether the squad has stopped to move while moving to attack outside home
	 * @return true if the squad is outside home and 'idle'
	 */
	bool hasHaltedAttack() const;

	bool mBig;
	std::vector<BWAPI::Unit*> mUnits;
	std::list<BWAPI::TilePosition> mCenter;
	AlliedSquadId mId;
	States mState;

	static utilities::KeyHandler<_AlliedSquadType>* mpsKeyHandler;
	static int mcsInstances;
};
}