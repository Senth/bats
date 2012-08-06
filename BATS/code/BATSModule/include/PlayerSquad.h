#pragma once

#include "TypeDefs.h"
#include "Utilities/KeyHandler.h"
#include "Utilities/KeyType.h"
#include "Config.h"
#include <BWAPI/TilePosition.h>
#include <vector>
#include <list>

// Forward declarations
namespace BWAPI {
	class Unit;
}


// Namespace for the project
namespace bats {

// Forward declarations
class GameTime;

/**
 * Common base class for virtual allied and enemy squads used for classifying those players.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class PlayerSquad : public config::OnConstantChangedListener {
public:
	/**
	 * Default constructor
	 */
	PlayerSquad();

	/**
	 * Destructor
	 */
	virtual ~PlayerSquad();

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
	 * Checks if the squad shall be updated. If it shall, then it updates the center position
	 * and calls updatedDerived() for derived classes.
	 * @note not a virtual function.
	 * @see updateDerived() for updates called in the derived classes
	 */
	void update();

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
	void addUnit(const BWAPI::Unit* pUnit);

	/**
	 * Removes a unit from the squad.
	 * @param pUnit the unit that shall be removed.
	 */
	void removeUnit(const BWAPI::Unit* pUnit);

	/**
	 * Returns the id of the squad.
	 * @return id of the squad
	 */
	PlayerSquadId getId() const;

	/**
	 * Prints graphical debug information, id of squad, number of units and
	 * number of supply in the center of the squad.
	 */
	virtual void printGraphicDebugInfo() const;

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

	/**
	 * Returns the direction of the squad, the direction is measured by the current center
	 * position and center position for measure_time seconds ago.
	 * @note the direction is not normalized.
	 * @return direction of the squad. TilePositions::Invalid if the squad doesn't have
	 * enough readings.
	 */
	BWAPI::TilePosition getDirection() const;

	/**
	 * Called when a constant has been updated (that this class listens to).
	 * Currently it only listens these variables:
	 * \code
	 * [classification.squad]
	 * measure_size
	 * \endcode
	 */
	virtual void onConstantChanged (config::ConstantName constantName);

	/**
	 * Checks if the specified unit belongs to this squad
	 * @param pUnit unit to check if it belongs to this squad
	 * @return true if the unit belongs to this squad
	 */
	bool belongsToThisSquad(BWAPI::Unit* pUnit) const;

protected:

	/**
	 * Updates the derived class.
	 */
	virtual void updateDerived() = 0;

	/**
	 * Returns the squared distance traveled since measure_size * measure_interval_time seconds ago.
	 * @return squared distance traveled, 0 if it the amount of measures is less than measure_size.
	 */
	int getDistanceTraveledSquared() const;

	/**
	 * Returns all units of the squad
	 * @return all units of the squad
	 */
	const std::vector<const BWAPI::Unit*>& getUnits() const;

	/**
	 * Checks if debug is off for the derived class (and thus this class)
	 * @return true if debug is off for derived class
	 */
	virtual bool isDebugOff() const = 0;

	/**
	 * Returns the debug string that shall be printed on the squad. By default this will
	 * print id, unit count, supply count. Implement this in the derived class to extend
	 * this functionality
	 * @return debug string that shall be printed on the squad.
	 */
	virtual std::string getDebugString() const;

	/**
	 * Checks if the squad has all the readings necessary to do some calculation based on
	 * the distance
	 * @return true if the squad has all readings done.
	 */
	bool isMeasureFull() const;

	static const GameTime* mpsGameTime;

private:
	/**
	 * Updates the center position
	 */
	void updateCenter();

	std::vector<const BWAPI::Unit*> mUnits;
	std::list<BWAPI::TilePosition> mCenter;
	PlayerSquadId mId;
	double mUpdateLast;

	static int mcsInstances;
	static utilities::KeyHandler<_PlayerSquadType>* mpsKeyHandler;
};
}