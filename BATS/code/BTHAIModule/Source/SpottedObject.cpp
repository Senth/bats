#include "SpottedObject.h"

using namespace BWAPI;
using namespace std;

SpottedObject::SpottedObject()
{
	position = Position(-1, -1);
	tilePosition = TilePosition(-1, -1);
	unitID = -1;
	active = true;
}

SpottedObject::SpottedObject(Unit* mUnit)
{
	type = mUnit->getType();
	position = mUnit->getPosition();
	tilePosition = mUnit->getTilePosition();
	unitID = mUnit->getID();
	active = true;
}

SpottedObject::SpottedObject(Position pos)
{
	position = pos;
	tilePosition = TilePosition(pos);
	type = UnitTypes::Zerg_Hive;
	unitID = 10101;
	active = true;
}

bool SpottedObject::isActive() const
{
	return active;
}

void SpottedObject::setInactive()
{
	active = false;
}

int SpottedObject::getUnitID() const
{
	return unitID;
}

UnitType SpottedObject::getType() const
{
	return type;
}

Position SpottedObject::getPosition() const
{
	return position;
}

TilePosition SpottedObject::getTilePosition() const
{
	return tilePosition;
}

bool SpottedObject::isAt(TilePosition tilePos) const
{
	if (tilePos.x() == tilePosition.x() && tilePos.y() == tilePosition.y())
	{
		return true;
	}
	return false;
}

double SpottedObject::getDistance(Position pos) const
{
	return position.getDistance(pos);
}

double SpottedObject::getDistance(TilePosition tilePos) const
{
	return tilePosition.getDistance(tilePos);
}
