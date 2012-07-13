#include "ExploreData.h"
#include "GameTime.h"
#include <cassert>
#include <BWTA.h>

using namespace bats;
using BWAPI::TilePosition;
using BWTA::Region;

ExploreData::ExploreData(const TilePosition& position, bool bExpansion) {
	mPosition = position;
	mbExpansion = bExpansion;
	mLastVisitFrame = 0;
}

ExploreData::~ExploreData() {
	// Does nothing
}

bool ExploreData::operator<(const ExploreData& rightExploreData) const {
	return mLastVisitFrame < rightExploreData.mLastVisitFrame;
}

const TilePosition& ExploreData::getCenterPosition() const {
	return mPosition;
}

bool ExploreData::matches(BWTA::Region* pRegion) const {
	if (pRegion == NULL) {
		return false;
	}
	BWAPI::TilePosition center = BWAPI::TilePosition(pRegion->getCenter());
	return matches(center);
}

bool ExploreData::matches(const BWAPI::TilePosition& center) const {
	// Inefficient
	//double dist = center.getDistance(mCenter);
	//if (dist <= 2) {
	//	return true;
	//}
	//return false;
	return mPosition == center;
}

bool ExploreData::isWithin(const BWAPI::TilePosition& position) const {
	BWTA::Region* pPositionRegion = BWTA::getRegion(position);
	if (pPositionRegion != NULL) {
		BWAPI::TilePosition regionCenter = BWAPI::TilePosition(pPositionRegion->getCenter());
		return regionCenter == mPosition;
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