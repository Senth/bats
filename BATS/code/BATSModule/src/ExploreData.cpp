#include "ExploreData.h"
#include "GameTime.h"
#include <cassert>
#include <BWTA.h>

using namespace bats;
using BWAPI::TilePosition;
using BWTA::Region;

ExploreData::ExploreData(const TilePosition& position, bool bExpansion) {
	mCenter = position;
	mbExpansion = bExpansion;
	mLastVisitFrame = 0;
}

ExploreData::~ExploreData() {
	// Does nothing
}

const TilePosition& ExploreData::getCenterPosition() const {
	return mCenter;
}

bool ExploreData::matches(BWTA::Region* region) const {
	if (region == NULL) {
		return false;
	}
	BWAPI::TilePosition center = BWAPI::TilePosition(region->getCenter());
	return matches(center);
}

bool ExploreData::matches(const BWAPI::TilePosition& center) const {
	// Inefficient
	//double dist = center.getDistance(mCenter);
	//if (dist <= 2) {
	//	return true;
	//}
	//return false;
	return mCenter == center;
}

bool ExploreData::isWithin(const BWAPI::TilePosition& position) const {
	BWTA::Region* pPositionRegion = BWTA::getRegion(position);
	if (pPositionRegion != NULL) {
		BWAPI::TilePosition regionCenter = BWAPI::TilePosition(pPositionRegion->getCenter());
		return regionCenter == mCenter;
	} else {
		return false;
	}
}

double ExploreData::secondsSinceLastVisit() const {
	return GameTime::getInstance()->getElapsedTime(mLastVisitFrame);
}

int ExploreData::getLastVisitFrame() const {
	return mLastVisitFrame;
}

void ExploreData::updateVisited() {
	mLastVisitFrame = BWAPI::Broodwar->getFrameCount();
}

bool ExploreData::isExpansion() const {
	return mbExpansion;
}