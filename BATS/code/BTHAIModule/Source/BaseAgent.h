#ifndef __BASEAGENT_H__
#define __BASEAGENT_H__

#include <windows.h>
#include <BWAPI.h>
#include <BWTA.h>
#include "BWTAExtern.h"
#include "Utilities/KeyType.h"
#include "BATSModule/include/TypeDefs.h"

/** The BaseAgent is the base agent class all agent classes directly or indirectly must extend. It contains some
 * common logic that is needed for several other agent implementations.
 * @todo move protected variables to private
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * 
 */
class BaseAgent {

private:
	bats::SquadId mSquadId;

protected:
	
	BWAPI::Unit* unit;
	BWAPI::UnitType type;
	BWAPI::TilePosition goal;
	int unitID;
	//int squadID;
	bool alive;
	int lastActionFrame;
	bool bBlock;
	std::string agentType;

	/**
	 * Returns true if the unit has any cooldown on either ground or air weapons
	 * @return true if the unit has cooldown on either ground or air weapons.
	 */
	bool isWeaponCooldown() const;

public:
	/** Constructor. */
	BaseAgent(BWAPI::Unit* mUnit);
	/** Destructor. */
	~BaseAgent();

	/**
	 * Set the squad id
	 * @param squadId the squad id to set
	 */ 
	void setSquadId(bats::SquadId squadId);

	/**
	 * Get Squad id
	 * @return the squad id
	 */
	const bats::SquadId& getSquadId() const;

	/** Called each update to issue orders. */
	virtual void computeActions()
	{
	}

	/** Used in debug modes to show a line to the agents' goal. */
	virtual void debug_showGoal()
	{
	}

	/** Returns the number of enemy units within weapon range of the agent. */
	int noUnitsInWeaponRange() const;

	/** Checks if this unit can attack the specified target unit. */
	bool canAttack(BWAPI::Unit* target) const;

	/** Checks if this unit can attack the specified unit type. */
	bool canAttack(BWAPI::UnitType type) const;

	/** Sets the goal for this unit. Goals are set from either the SquadCommander for attacking
	 * or defending units, or from ExplorationManager for explore units. */
	virtual void setGoal(BWAPI::TilePosition goal);

	/** Clears the goal for this unit. */
	virtual void clearGoal();

	/** Returns the current goal for this unit. */
	const BWAPI::TilePosition& getGoal() const;

	/** Sets the current frame when this agent is issued an order. */
	void setActionFrame();

	/** Returns the last frame this agent was issued an order. */
	int getLastActionFrame() const;

	/** Returns the unique type name for the agent type. */
	const std::string& getTypeName() const;

#pragma warning (push)
#pragma warning (disable:4100)
	/** Assigns the agent to build the specified type of unit. */
	virtual bool assignToBuild(BWAPI::UnitType type)
	{
		return false;
	}

	/** Assigns the agent to repair a unit. */
	virtual bool assignToRepair(BWAPI::Unit* building)
	{
		return false;
	}

	/** Assigns the agent to continue building a non-finished building. */
	virtual bool assignToFinishBuild(BWAPI::Unit* building)
	{
		return false;
	}
#pragma warning (pop)

	/** Assigns this agent to explore the game world. */
	virtual void assignToExplore()
	{
	}

	/** Assigns this agent to be in the attack force. */
	virtual void assignToAttack()
	{
	}

	/** Assigns this agent to defend the base. */
	virtual void assignToDefend()
	{
	}

	/** Returns true if this agent is used to explore the game world. */
	virtual bool isExploring() const
	{
		return false;
	}

	/** Returns true if this agent is used to attack the enemy. */
	virtual bool isAttacking() const
	{
		return false;
	}

	/** Returns true if this agent is defending the own base. */
	virtual bool isDefending() const
	{
		return false;
	}

	/** Returns true if the unit is currently being built */
	bool isBeingBuilt() const;

	/** Used to print info about this agent to the screen. */
	virtual void printInfo() const;

	/** Returns true if this agent can build units of the specified type. */
	virtual bool canBuild(BWAPI::UnitType type) const;

	/** Returns the unique id for this agent. Agent id is the same as the id of the unit
	 * assigned to the agent. */
	int getUnitID() const;

	/** Returns the type for the unit handled by this agent. */
	const BWAPI::UnitType& getUnitType() const;

	/** Returns a reference to the unit assigned to this agent. */
	BWAPI::Unit* getUnit();

	/**
	 * Returns a const pointer to the unit assigned to this agent.
	 */
	const BWAPI::Unit* getUnit() const;

	/** Called when the unit assigned to this agent is destroyed. */
	virtual void destroyed();

	/** Returns true if this agent is active, i.e. the unit is not destroyed. */
	bool isAlive() const;

	/** Returns true if the specified unit is the same unit assigned to this agent. */
	bool matches(BWAPI::Unit* mUnit) const;

	/** Returns true if the agent is of the specified type. */
	bool isOfType(BWAPI::UnitType type) const;

	/** Returns true if the unit is of the specified type. */
	static bool isOfType(BWAPI::Unit* mUnit, BWAPI::UnitType type);

	/** Returns true if mType is the same UnitType as toCheckType. */
	static bool isOfType(BWAPI::UnitType mType, BWAPI::UnitType toCheckType);

	/** Checks if there are any enemy detector units withing range of the
	 * specified position. True if there is, false if not. */
	bool isDetectorWithinRange(BWAPI::TilePosition pos, int range) const;

	/** Returns true if this agent is a building. */
	bool isBuilding() const;

	/** Returns true if this agent is a worker. */
	bool isWorker() const;

	/** Returns true if this agent is a free worker, i.e. is idle or is gathering minerals. */
	bool isFreeWorker() const;

	/** Returns true if this agent is a combat unit. */
	bool isUnit() const;

	/** Returns true if this agent is under attack, i.e. lost hitpoints since last check. */
	bool isUnderAttack() const;

	/** Returns true if this agent is damaged. */
	bool isDamaged() const;

	/** Orders the Terran Comsat Station to do a Scanner Sweep at the specified area. */
	bool doScannerSweep(BWAPI::TilePosition pos);

	/** Orders the Zerg Queen to do an Ensnare at the specified area. */
	static bool doEnsnare(BWAPI::TilePosition pos);

	///** Assigns this agent to the squad with the specified id. */
	//void _deprecated_setSquadID(int id);

	///** Returns the squad this agent belongs to, or -1 if it doesnt
	// * belong to any squad. */
	//int _deprecated_getSquadID();

};

#endif
