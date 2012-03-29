#include "Resource.h"
#include <BWAPI/Unit.h>
#include <BWAPI/Game.h>
#include <Utilities/Logger.h>

using namespace BWAPI;
using namespace bats;

Resource::Resource(BWAPI::Unit* pUnit) :
	M_INITIAL(pUnit->getInitialResources()),
	M_UNIT_ID(pUnit->getID()),
	M_POSITION(pUnit->getTilePosition()),
	mCurrent(pUnit->getResources())
{
	// Does nothing
}

Resource::~Resource() {
	// Does nothing
}

int Resource::getCurrent() const {
	return mCurrent;
}

int Resource::getInitial() const {
	return M_INITIAL;
}

int Resource::getId() const {
	return M_UNIT_ID;
}

const TilePosition& Resource::getPosition() const {
	return M_POSITION;
}

void Resource::update() {
	Unit* pUnit = Broodwar->getUnit(M_UNIT_ID);

	DEBUG_MESSAGE_CONDITION(NULL == pUnit, utilities::LogLevel_Warning,
		"Resource::update() | unit is NULL, where is the unit. Can't we get units that aren't " <<
		" visible?");

	if (NULL != pUnit && pUnit->isVisible()) {
		mCurrent = pUnit->getResources();
	}
}

Resource& Resource::operator =(const Resource& resource) {
	const_cast<int&>(M_UNIT_ID) = resource.M_UNIT_ID;
	const_cast<TilePosition&>(M_POSITION) = resource.M_POSITION;
	const_cast<int&>(M_INITIAL) = resource.M_INITIAL;
	mCurrent = resource.mCurrent;
	return *this;
}