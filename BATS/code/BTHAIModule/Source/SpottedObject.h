#pragma once

#include <BWAPI.h>
#include <BWTA.h>

/** The SpottedObject class is a help class for the ExplorationManager.
 ** It contains all details about a spotted enemy unit or neutral resource. 
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (senth.wallace@gmail.com)
 * Now using const and references, and changed to TilePositions::Invalid default value.
 */
class SpottedObject {

private:
	BWAPI::UnitType mType;
	BWAPI::Position mPosition;
	BWAPI::TilePosition mTilePosition;
	int mUnitId;
	bool mActive;

public:
	/** Default constructor. Should not be used. */
	SpottedObject();

	/** Creates an object from a unit reference. */
	SpottedObject(const BWAPI::Unit* unit);

	/** Creates a spotted object from an interesting position. */
	SpottedObject(const BWAPI::Position& pos);

	/** Returns true if this SpottedObject is active, false if not. */
	bool isActive() const;

	/** Sets this SpottedObject as inactive, i.e. it is probably destroyed. */
	void setInactive();

	/** Returns the unique id of the spotted unit. */
	int getUnitID() const;

	/** Returns the type of the spotted unit. */
	const BWAPI::UnitType& getType() const;

	/** Returns the position of the spotted unit. */
	const BWAPI::Position& getPosition() const;

	/** Returns the tile position of the spotted unit. */
	const BWAPI::TilePosition& getTilePosition() const;

	/** Returns true if the SpottedObject is at this TilePosition. */
	bool isAt(const BWAPI::TilePosition& tilePos) const;

};