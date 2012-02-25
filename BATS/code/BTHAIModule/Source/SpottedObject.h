#ifndef __SPOTTEDOBJECT_H__
#define __SPOTTEDOBJECT_H__

#include <BWAPI.h>
#include <BWTA.h>

/** The SpottedObject class is a help class for the ExplorationManager. It contains all details about a spotted
 * enemy unit or neutral resource. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class SpottedObject {

private:
	BWAPI::UnitType type;
	BWAPI::Position position;
	BWAPI::TilePosition tilePosition;
	int unitID;
	bool active;

public:
	/** Default constructor. Should not be used. */
	SpottedObject();

	/** Creates an object from a unit reference. */
	SpottedObject(BWAPI::Unit* mUnit);

	/** Creates a spotted object from an interesting position. */
	SpottedObject(BWAPI::Position pos);

	/** Returns true if this SpottedObject is active, false if not. */
	bool isActive();

	/** Sets this SpottedObject as inactive, i.e. it is probably destroyed. */
	void setInactive();

	/** Returns the unique id of the spotted unit. */
	int getUnitID();

	/** Returns the type of the spotted unit. */
	BWAPI::UnitType getType();

	/** Returns the position of the spotted unit. */
	BWAPI::Position getPosition();

	/** Returns the tileposition of the spotted unit. */
	BWAPI::TilePosition getTilePosition();

	/** Returns true if the SpottedObject is at this TilePosition. */
	bool isAt(BWAPI::TilePosition tilePos);

	/** Returns the distance between the spotted object and a position */
	double getDistance(BWAPI::Position pos);

	/** Returns the distance between the spotted object and a tileposition */
	double getDistance(BWAPI::TilePosition tilePos);

};

#endif
