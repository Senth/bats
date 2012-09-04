#include "CoverMap.h"
#include "AgentManager.h"
#include "BaseAgent.h"
#include "BWTAExtern.h"
#include "Profiler.h"
#include "BatsModule/include/BuildPlanner.h"
#include "BatsModule/include/ExplorationManager.h"
#include "BatsModule/include/Config.h"
#include "BatsModule/include/UnitHelper.h"
#include "BatsModule/include/Helper.h"
#include "utilities/Logger.h"
#include <cassert>
#include <BWTA.h>
#include <BWAPI/Game.h>

using namespace BWAPI;
using namespace BWTA;
using namespace std;

CoverMap* CoverMap::msInstance = NULL;

CoverMap::CoverMap()
{
	mExplorationManager = NULL;
	mExplorationManager = bats::ExplorationManager::getInstance();

	mMapWidth = Broodwar->mapWidth();
	mMapHeight = Broodwar->mapHeight();
	mRange = 30;
	initCoverMap();
	mapData = MapDataReader();
	mapData.readMap();
	initTilePositionRegions();
}

CoverMap::~CoverMap()
{
	for(int i = 0 ; i < mMapWidth ; i++)
	{
		delete[] mCoverMap[i];
	}
	delete[] mCoverMap;

	msInstance = NULL;
	mExplorationManager = NULL;
}

CoverMap* CoverMap::getInstance()
{
	if (NULL == msInstance)
	{
		msInstance = new CoverMap();
	}
	return msInstance;
}

void CoverMap::initCoverMap() {
	mCoverMap = new TileStates*[mMapWidth];
	for(int x = 0 ; x < mMapWidth ; x++) {
		mCoverMap[x] = new TileStates[mMapHeight];
	}

	// Fill from static map, if the region is ok or not.
	for(int x = 0 ; x < mMapWidth ; x++) {
		for (int y = 0; y < mMapHeight; y++) {
			if (Broodwar->isBuildable(x, y)) {
				mCoverMap[x][y] = TileState_Buildable;
			} else {
				// Pad outwards so that we don't get small spaces
				Corners corner(TilePosition(x, y), 1);
				fill(corner, TileState_Blocked);
			}
		}
	}

	//Fill from current agents
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++) {
		BaseAgent* agent = agents[i];
		if (agent->isBuilding())
		{
			Corners c = getCorners(agent->getUnit());
			fill(c);
		}
	}

	//Fill from minerals
	for(set<Unit*>::iterator mineralIt = Broodwar->getMinerals().begin(); mineralIt != Broodwar->getMinerals().end(); ++mineralIt) {
		Corners corners;
		const TilePosition& mineralPos = (*mineralIt)->getTilePosition();
		corners.xMin = mineralPos.x() - 1;
		corners.yMin = mineralPos.y() - 1;
		corners.xMax = mineralPos.x() + 2;
		corners.yMax = mineralPos.y() + 1;
		fill(corners, TileState_Mineral);
	}

	//Fill from gas
	for(set<Unit*>::iterator gasIt = Broodwar->getGeysers().begin(); gasIt != Broodwar->getGeysers().end(); ++gasIt) {
		Corners corners;
		const TilePosition& gasPos = (*gasIt)->getTilePosition();
		corners.xMin = gasPos.x() - 1;
		corners.yMin = gasPos.y() - 1;
		corners.xMax = gasPos.x() + 4;
		corners.yMax = gasPos.y() + 2;
		fill(corners, TileState_Blocked);
		mCoverMap[gasPos.x()][gasPos.y()] = TileState_Gas;
	}

	
	//if (::analyzed) {
		//Fill from narrow chokepoints
		const set<BWTA::Region*>& regions = BWTA::getRegions();
		set<BWTA::Region*>::const_iterator regionIt;
		for(regionIt = regions.begin() ; regionIt != regions.end() ; ++regionIt) {
			const set<BWTA::Chokepoint*>& chokepoints = (*regionIt)->getChokepoints();
			set<BWTA::Chokepoint*>::const_iterator chokeIt;
			for(chokeIt = chokepoints.begin() ; chokeIt != chokepoints.end(); ++chokeIt) {
				if ((*chokeIt)->getWidth() <= 5 * 32) {
					TilePosition center = TilePosition((*chokeIt)->getCenter());
					Corners corners(center, 3);
					fill(corners);
				}
			}
		}

		// Reserve expansion positions
		const Corners& baseTypeCorners = getCorners(BWAPI::Broodwar->self()->getRace().getCenter());
		const set<BWTA::BaseLocation*> baseLocations = BWTA::getBaseLocations();
		set<BWTA::BaseLocation*>::const_iterator baseIt;
		for (baseIt = baseLocations.begin(); baseIt != baseLocations.end(); ++baseIt) {
			const TilePosition& basePos = (*baseIt)->getTilePosition();
			Corners currentCorners = baseTypeCorners + basePos;

			if (isAreaFree(currentCorners)) {
				fill(currentCorners, TileState_ExpansionReserved);
			}
		}
	//}
}

void CoverMap::initTilePositionRegions() {
	for (int x = 0; x < mMapWidth; ++x) {
		for (int y = 0; y < mMapHeight; ++y) {
			TilePosition currentPos(x,y);
			BWTA::Region* region = BWTA::getRegion(currentPos);

			if (NULL != region) {
				mRegionTiles.insert(make_pair(region, currentPos));
			}
		}
	}
}

bool CoverMap::isAreaFree(const Corners& corners) const {
	for (int x = corners.xMin; x <= corners.xMax; ++x) {
		for (int y = corners.yMin; y <= corners.yMax; ++y) {
			if (!isPositionFree(TilePosition(x,y))) {
				return false;
			}
		}
	}

	return true;
}

bool CoverMap::isPositionFree(const TilePosition& pos) const {
	if (pos.x() >= 0 && pos.x() < mMapWidth && pos.y() >= 0 && pos.y() < mMapHeight) {
		if (mCoverMap[pos.x()][pos.y()] == TileState_Buildable) {
			return true;
		}
	}
	return false;
}

void CoverMap::blockPosition(const TilePosition& buildSpot) {
	if (buildSpot != TilePositions::Invalid) {
		//Error check
		return;
	}
	mCoverMap[buildSpot.x()][buildSpot.y()] = TileState_Blocked;
}

bool CoverMap::canBuildAt(const UnitType& toBuild, const TilePosition& buildSpot, int builderId) const
{
	const Corners& corners = getCorners(toBuild, buildSpot);

	// Check cover map.
	for (int x = corners.xMin; x <= corners.xMax; x++) {
		if (x >= 0 && x < mMapWidth ) {
			for (int y = corners.yMin; y <= corners.yMax; y++) {
				if (x >= 0 && x < mMapWidth && y >= 0 && y < mMapHeight) {
					//Cant build here.
					if (mCoverMap[x][y] != TileState_Buildable) {
						// But maybe the expansion can?
						if (!toBuild.isResourceDepot() || mCoverMap[x][y] != TileState_ExpansionReserved) {
							return false;
						}
					}
				}
				// Out of bounds
				else {
					return false;
				}
			}
		}
		// Out of bounds
		else {
			return false;
		}
	}


	// Is the build spot in an allied region (but not in our region)
	// Do not use for bases
	const BWTA::Region* region = BWTA::getRegion(buildSpot);
	if (!toBuild.isResourceDepot()) {
		if (isRegionOccupiedByOurTeam(region, false, true) && !isRegionOccupiedByOurTeam(region, true, false)) {
			return false;
		}
	}


	// Check if path is available
	if (!mExplorationManager->canReach(Broodwar->self()->getStartLocation(), buildSpot))
	{
		return false;
	}


	// Check any units on tile (only check the actual build tiles and not the entire cover area).
	const vector<Unit*> teamUnits = bats::UnitHelper::getTeamUnits();
	TilePosition maxPos = buildSpot;
	maxPos.x() += toBuild.tileWidth() - 1;
	maxPos.y() += toBuild.tileHeight() - 1;
	if (bats::UnitHelper::unitsInArea(teamUnits, buildSpot, maxPos, builderId))
	{
		return false;
	}


	// If Protoss, check PSI coverage
	if (bats::BuildPlanner::isProtoss())
	{
		if (toBuild.requiresPsi())
		{
			if (!Broodwar->hasPower(buildSpot, toBuild.tileWidth(), toBuild.tileHeight()))
			{
				return false;
			}
		}
	}


	// If Zerg, check creep
	if (bats::BuildPlanner::isZerg())
	{
		if (toBuild == UnitTypes::Zerg_Hatchery)
		{
			//Do not build if we have creep (to spread creep out)
			if (Broodwar->hasCreep(buildSpot))
			{
				return false;
			}
		}
		else if (toBuild.requiresCreep())
		{
			if (!Broodwar->hasCreep(buildSpot))
			{
				return false;
			}
		}
	}

	//All passed. It is possible to build here.
	return true;
}

TilePosition CoverMap::findBuildSpot(const UnitType& toBuild, int builderId) const
{
	//Refinery
	if (toBuild.isRefinery())
	{
		//Use refinery method
		return findRefineryBuildSpot();
	}

	//If we find unpowered buildings, build a Pylon there
	if (toBuild == UnitTypes::Protoss_Pylon)
	{
		const vector<BaseAgent*>& agents = AgentManager::getInstance()->getAgents();
		for (size_t i = 0; i < agents.size(); i++)
		{
			BaseAgent* agent = agents[i];
			if (agent->isAlive())
			{
				Unit* cUnit = agent->getUnit();
				if (cUnit->isUnpowered())
				{
					//Broodwar->printf("Build pylon at unpowered building %s", cUnit->getType().getName().c_str());
					return findBuildSpot(toBuild, cUnit->getTilePosition(), builderId);
				}
			}
		}

		if (AgentManager::getInstance()->countNoUnits(UnitTypes::Protoss_Pylon) > 0)
		{
			TilePosition cp = findChokepoint();
			if (cp != TilePositions::Invalid)
			{
				if (!Broodwar->hasPower(cp, UnitTypes::Protoss_Cybernetics_Core))
				{
					if (AgentManager::getInstance()->noInProduction(UnitTypes::Protoss_Pylon) == 0)
					{
						TilePosition spot = findBuildSpot(toBuild, cp, builderId);
						return spot;
					}
				}
			}
		}
	}


	/// @todo Only build bunker, photon cannon, creep colon near chokepoints and in our region...
	//Build near chokepoints: Bunker, Photon Cannon, Creep Colony
	if (toBuild == UnitTypes::Terran_Bunker ||
		toBuild == UnitTypes::Protoss_Photon_Cannon ||
		toBuild == UnitTypes::Zerg_Creep_Colony)
	{
		TilePosition chokepoint = findChokepoint();
		if (chokepoint != TilePositions::Invalid) {
			TilePosition spot = findBuildSpot(toBuild, chokepoint, builderId);
			return spot;
		}
	}


	//Base buildings.
	if (toBuild.isResourceDepot()) {
		TilePosition start = findExpansionSite();
		if (start != TilePositions::Invalid) {
			return findBuildSpot(toBuild, start, builderId);
		}
		else {
			//No expansion site found.
			return TilePositions::Invalid;
		}
	}


	// General buildings. Find a location in a region we already have
	const set<BWTA::Region*>& regions = BWTA::getRegions();
	set<BWTA::Region*>::const_iterator regionIt;
	for (regionIt = regions.begin(); regionIt != regions.end(); ++regionIt) {
		if (isRegionOccupiedByOurTeam(*regionIt, true, false)) {
			const TilePosition& buildSpot = findBuildSpotInRegion(toBuild, *regionIt, builderId);
			if (buildSpot != TilePositions::Invalid) {
				return buildSpot;
			}
		}
	}

	// Did not find any build spot in our regions, find build spot in a close region
	// But not in an allied region.
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = agents.size() - 1; i >= 0; --i) {
		BaseAgent* agent = agents[i];
		if (agent->isAlive() && agent->isBuilding()) {
			TilePosition start = agent->getUnit()->getTilePosition();
			TilePosition buildSpot = findBuildSpot(toBuild, start, builderId);
			if (buildSpot != TilePositions::Invalid) {
				return buildSpot;
			}
		}
	}

	return TilePositions::Invalid;
}

TilePosition CoverMap::findBuildSpotInRegion(const BWAPI::UnitType& unitType, const BWTA::Region* region, int builderId) const {
	TilePosition buildSpot = TilePositions::Invalid;
	pair<RegionTileMap::const_iterator, RegionTileMap::const_iterator> tileRange = mRegionTiles.equal_range(region);
	RegionTileMap::const_iterator tileIt = tileRange.first;
	while (buildSpot == TilePositions::Invalid && tileIt != tileRange.second) {
		if (canBuildAt(unitType, tileIt->second, builderId)) {
			buildSpot = tileIt->second;
		}

		++tileIt;
	}

	return buildSpot;
}

TilePosition CoverMap::findSpotAtSide(const UnitType& toBuild, const TilePosition& start, const TilePosition& end, int builderId) const
{
	int dX = end.x() - start.x();
	if (dX != 0) dX = 1;
	int dY = end.y() - start.y();
	if (dY != 0) dY = 1;

	TilePosition cPos = start;
	bool done = false;
	while (!done) {
		if (canBuildAt(toBuild, cPos, builderId)) {
			return cPos;
		}
		int cX = cPos.x() + dX;
		int cY = cPos.y() + dY;
		cPos = TilePosition(cX, cY);
		if (cPos == end) {
			done = true;
		}
	}

	return TilePositions::Invalid;
}

bool CoverMap::isOccupied(const BWTA::Region* region) const
{
	const BWTA::Polygon& regionPolygon = region->getPolygon();
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++) {
		BaseAgent* agent = agents[i];
		if (agent->isAlive() && (agent->isBuilding() && !agent->isOfType(UnitTypes::Protoss_Pylon))) {
			BWTA::Region* aRegion = getRegion(agent->getUnit()->getTilePosition());
			if (aRegion == region) {
				return true;
			}
		}
	}

	/// @todo Check for the next expansion site to see if has expanded. This information shall
	/// be available in the commander later, not the Exploration manager. Maybe creating
	/// an ExpansionManager?


	return false;
}

double CoverMap::getChokepointPrio(const TilePosition& center) const
{
	TilePosition ePos = mExplorationManager->getClosestSpottedBuilding(center).first;

	if (ePos != TilePositions::Invalid) {
		double dist = ePos.getDistance(center);
		return 1000 - dist;
	}
	else
	{
		double dist = Broodwar->self()->getStartLocation().getDistance(center);
		return dist;
	}
}

TilePosition CoverMap::findChokepoint() const {
	double bestPrio = -1;
	Chokepoint* bestChoke = NULL;

	set<BWTA::Region*>::const_iterator regionIt;
	for(regionIt = getRegions().begin(); regionIt != getRegions().end(); ++regionIt)
	{
		if (isOccupied((*regionIt)))
		{
			set<Chokepoint*>::const_iterator chokeIt;
			for(chokeIt = (*regionIt)->getChokepoints().begin(); chokeIt != (*regionIt)->getChokepoints().end(); ++chokeIt)
			{
				if (isEdgeChokepoint(*chokeIt))
				{
					double cPrio = getChokepointPrio(TilePosition((*chokeIt)->getCenter()));
					if (cPrio > bestPrio)
					{
						bestPrio = cPrio;
						bestChoke = *chokeIt;
					}
				}
			}
		}
	}

	TilePosition guardPos = Broodwar->self()->getStartLocation();
	if (bestChoke != NULL)
	{
		guardPos = findDefensePos(bestChoke);
	}

	return guardPos;
}

bool CoverMap::isEdgeChokepoint(const Chokepoint* choke) const
{
	pair<BWTA::Region*,BWTA::Region*> regions = choke->getRegions();
	//If both is occupied it is not an edge chokepoint
	if (isOccupied(regions.first) && isOccupied(regions.second))
	{
		return false;
	}
	//...but one of them must be occupied
	if (isOccupied(regions.first) || isOccupied(regions.second))
	{
		return true;
	}
	return false;
}

TilePosition CoverMap::findDefensePos(const Chokepoint* choke) const
{
	TilePosition defPos = TilePosition(choke->getCenter());
	TilePosition chokePos = defPos;

	double size = choke->getWidth();
	if (size <= 32 * 3)
	{
		//Very narrow chokepoint, dont crowd it
		double bestDist = 1000;
		TilePosition basePos = Broodwar->self()->getStartLocation();

		int maxD = 8;
		int minD = 5;

		// Find a good place to defend the chokepoint
		for (int cX = chokePos.x() - maxD; cX <= chokePos.x() + maxD; cX++)
		{
			for (int cY = chokePos.y() - maxD; cY <= chokePos.y() + maxD; cY++)
			{
				TilePosition cPos = TilePosition(cX, cY);
				if (mExplorationManager->canReach(basePos, cPos))
				{
					double chokeDist = chokePos.getDistance(cPos);
					double baseDist = basePos.getDistance(cPos);

					if (chokeDist >= minD && chokeDist <= maxD)
					{
						if (baseDist < bestDist)
						{
							bestDist = baseDist;
							defPos = cPos;
						}
					}
				}
			}
		}
	}

	//Uncomment to make defenders crowd around defensive structures.
	/*UnitType defType;
	if (BuildPlanner::isZerg()) defType = UnitTypes::Zerg_Sunken_Colony;
	if (BuildPlanner::isProtoss()) defType = UnitTypes::Protoss_Photon_Cannon;
	if (BuildPlanner::isTerran()) defType = UnitTypes::Terran_Bunker;

	BaseAgent* turret = AgentManager::getInstance()->getClosestAgent(defPos, defType);
	if (turret != NULL)
	{
		TilePosition tPos = turret->getUnit()->getTilePosition();
		double dist = tPos.getDistance(defPos);
		if (dist <= 12)
		{
			defPos = tPos;
		}
	}*/

	return defPos;
}

bool CoverMap::isRegionOccupiedByOurTeam(const BWTA::Region* region, bool includeOurStructures, bool includeAlliedStructures) {
	if (NULL == region) {
		return false;
	}

	set<Player*> teamPlayers;
	if (includeAlliedStructures) {
		teamPlayers = Broodwar->allies();
	}
	if (includeOurStructures) {
		teamPlayers.insert(Broodwar->self());
	}

	const Position regionToCheckPos = region->getCenter();

	set<Player*>::const_iterator playerIt;
	for (playerIt = teamPlayers.begin(); playerIt != teamPlayers.end(); ++playerIt) {
		const set<Unit*>& units = (*playerIt)->getUnits();
		set<Unit*>::const_iterator unitIt;
		for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
			Unit* unit = (*unitIt);
			if (unit->getType().isBuilding()) {
				BWTA::Region* unitRegion = BWTA::getRegion(unit->getPosition());
				const Position& unitRegionPos = unitRegion->getCenter();

				if (unitRegionPos == regionToCheckPos) {
					return true;
				}
			}
		}
	}

	return false;
}

TilePosition CoverMap::findBuildSpot(const UnitType& toBuild, const TilePosition& start, int builderId) const
{
	//Check start pos
	if (canBuildAt(toBuild, start, builderId)) return start;

	//Search outwards
	int cDiff = 1;
	TilePosition spot = TilePositions::Invalid;
	while (true) 
	{
		//Top
		TilePosition s = TilePosition(start.x() - cDiff, start.y() - cDiff);
		TilePosition e = TilePosition(start.x() + cDiff, start.y() - cDiff);
		spot = findSpotAtSide(toBuild, s, e, builderId);
		if (spot != TilePositions::Invalid)
		{
			break;
		}

		//Bottom
		s = TilePosition(start.x() - cDiff, start.y() + cDiff);
		e = TilePosition(start.x() + cDiff, start.y() + cDiff);
		spot = findSpotAtSide(toBuild, s, e, builderId);
		if (spot != TilePositions::Invalid)
		{
			break;
		}

		//Left
		s = TilePosition(start.x() - cDiff, start.y() - cDiff);
		e = TilePosition(start.x() - cDiff, start.y() + cDiff);
		spot = findSpotAtSide(toBuild, s, e, builderId);
		if (spot != TilePositions::Invalid)
		{
			break;
		}

		//Right
		s = TilePosition(start.x() + cDiff, start.y() - cDiff);
		e = TilePosition(start.x() + cDiff, start.y() + cDiff);
		spot = findSpotAtSide(toBuild, s, e, builderId);
		if (spot != TilePositions::Invalid)
		{
			break;
		}

		cDiff++;
		if (cDiff > mRange) {
			break;
		}
	}
	
	return spot;
}

void CoverMap::addConstructedBuilding(const Unit* unit)
{
	if (unit->getType().isAddon())
	{
		//Addons are handled by their main buildings
		return;
	}

	const Corners& corners = getCorners(unit);
	fill(corners);
}

void CoverMap::buildingDestroyed(const Unit* unit)
{
	if (unit->getType().isAddon())
	{
		//Addons are handled by their main buildings
		return;
	}

	Corners corners = getCorners(unit);
	clear(corners);
}

void CoverMap::fill(const Corners& corners, TileStates tileState) {
	for (int x = corners.xMin; x <= corners.xMax; x++) {
		for (int y = corners.yMin; y <= corners.yMax; y++) {
			if (x >= 0 && x < mMapWidth && y >= 0 && y < mMapHeight) {
				mCoverMap[x][y] = tileState;
			}
		}
	}
}

void CoverMap::fillTemp(const UnitType& toBuild, const TilePosition& buildSpot) {
	const Corners& corners = getCorners(toBuild, buildSpot);

	for (int x = corners.xMin; x <= corners.xMax; x++) {
		for (int y = corners.yMin; y <= corners.yMax; y++) {
			if (x >= 0 && x < mMapWidth && y >= 0 && y < mMapHeight) {
				if (mCoverMap[x][y] == TileState_Buildable) {
					mCoverMap[x][y] = TileState_TempBlocked;
				}
			}
		}
	}
}

void CoverMap::clear(const Corners& corners) {
	CoverMap::fill(corners, TileState_Buildable);
}

void CoverMap::clearTemp(const UnitType& toBuild, const TilePosition& buildSpot)
{
	if (buildSpot == TilePositions::Invalid){
		return;
	}

	const Corners& corners = getCorners(toBuild, buildSpot);

	for (int x = corners.xMin; x <= corners.xMax; x++) {
		for (int y = corners.yMin; y <= corners.yMax; y++) {
			if (x >= 0 && x < mMapWidth && y >= 0 && y < mMapHeight) {
				if (mCoverMap[x][y] == TileState_TempBlocked) {
					mCoverMap[x][y] = TileState_Buildable;
				}
			}
		}
	}
}

Corners CoverMap::getCorners(const Unit* unit) const
{
	return getCorners(unit->getType(), unit->getTilePosition());
}

Corners CoverMap::getCorners(const UnitType& type, const TilePosition& center) const
{
	int x1 = center.x();
	int y1 = center.y();
	int x2 = x1 + type.tileWidth() - 1;
	int y2 = y1 + type.tileHeight() - 1;

	int margin = 1;
	if (type.canProduce()) {
		margin = 1;
	}
	else if (type == UnitTypes::Terran_Supply_Depot) {
		margin = 1;
	}
	else if (type == UnitTypes::Protoss_Pylon) {
		margin = 0;
	}

	x1 -= margin;
	x2 += margin;
	y1 -= margin;
	y2 += margin;

	//Special case: Terran Addon buildings
	//Add 1 extra spaces to the right to make space for the addons.
	//This means a building could be placed directly to the right of the addon.
	if (type == UnitTypes::Terran_Factory ||
		type == UnitTypes::Terran_Starport ||
		type == UnitTypes::Terran_Command_Center ||
		type == UnitTypes::Terran_Science_Facility)
	{
		x2 += 1;
	}

	Corners c;
	c.xMin = x1;
	c.yMin = y1;
	c.xMax = x2;
	c.yMax = y2;

	return c;
}

TilePosition CoverMap::findRefineryBuildSpot() const {
	TilePosition buildSpot = findClosestGasWithoutRefinery();
	if (buildSpot != TilePositions::Invalid) {
		BaseAgent* base = AgentManager::getInstance()->getClosestBase(buildSpot);
		if (base == NULL) {
			DEBUG_MESSAGE(utilities::LogLevel_Warning, "CoverMap::findRefineryBuildSpot(): No base found!");
			return TilePositions::Invalid;
		}
		else
		{
			double dist = buildSpot.getDistance(base->getUnit()->getTilePosition());
			if (dist >= 13) 
			{
				DEBUG_MESSAGE(utilities::LogLevel_Finer, "CoverMap::findRefineryBuildSpot(): Base too far away ("
					<< dist << " tiles)"
				);
				return TilePositions::Invalid;
			}
		}

	}
	return buildSpot;
}

TilePosition CoverMap::findClosestGasWithoutRefinery() const {
	TilePosition bestSpot = TilePositions::Invalid;
	int bestDist = INT_MAX;
	TilePosition home = Broodwar->self()->getStartLocation();

	for(int x = 0 ; x < mMapWidth ; x++) {
		for (int y = 0; y < mMapHeight; y++) {
			if (mCoverMap[x][y] == TileState_Gas) {
				TilePosition cPos(x,y);

				bool ok = true;
				vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
				for (size_t i = 0; i < agents.size(); i++) {
					Unit* unit = agents[i]->getUnit();
					if (unit->getType().isRefinery()) {
						int dist = bats::getSquaredDistance(unit->getTilePosition(), cPos);
						if (dist <= 4) {
							ok = false;
						}
					}
				}

				if (ok) {
					if (mExplorationManager->canReach(home, cPos)) {
						BaseAgent* agent = AgentManager::getInstance()->getClosestBase(cPos);
						int dist = bats::getSquaredDistance(agent->getUnit()->getTilePosition(), cPos);
						if (dist < bestDist) {
							bestDist = dist;
							bestSpot = cPos;
						}
					}
				}
			}
		}
	}

	return bestSpot;
}

TilePosition CoverMap::findExpansionSite() const
{
	UnitType baseType = Broodwar->self()->getRace().getCenter();
	double bestDist = 100000;
	TilePosition bestPos = TilePositions::Invalid;
	
	//Iterate through all base locations
	for(set<BWTA::BaseLocation*>::const_iterator baseIt = BWTA::getBaseLocations().begin(); baseIt != BWTA::getBaseLocations().end(); baseIt++)
	{
		TilePosition pos = (*baseIt)->getTilePosition();
		bool taken = false;
		
		//Check if own buildings are close
		vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
		int noBases = 0;
		for (size_t i = 0; i < agents.size(); i++)
		{
			BaseAgent* agent = agents[i];
			if (agent->isAlive() && agent->getUnitType().isResourceDepot())
			{
				double dist = pos.getDistance(agent->getUnit()->getTilePosition());
				if (dist <= 12)
				{
					noBases++;
				}
			}
		}
		if (bats::BuildPlanner::isZerg())
		{
			if (noBases >= 2) taken = true;
		}
		else
		{
			if (noBases >= 1) taken = true;
		}

		//Check if enemy buildings are close
		int eCnt = mExplorationManager->countSpottedBuildingsWithinRange(pos, 20);
		if (eCnt > 0)
		{
			taken = true;
		}

		//Not taken, calculate ground distance
		if (!taken)
		{
			if (mExplorationManager->canReach(Broodwar->self()->getStartLocation(), pos))
			{
				double ourDistance;
				ourDistance = mapData.getDistance(Broodwar->self()->getStartLocation(), pos);
				if (ourDistance <= bestDist)
				{
					bestDist = ourDistance;
					bestPos = pos;
				}
			}
		}
	}

	return bestPos;
}

Unit* CoverMap::findClosestMineral(const TilePosition& workerPos) const
{
	Unit* mineral = NULL;
	double bestDist = 10000;

	for(set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin(); i!= BWTA::getBaseLocations().end(); i++)
	{
		TilePosition pos = (*i)->getTilePosition();
		double cDist = pos.getDistance(workerPos);
		if (cDist < bestDist)
		{
			//Find closest base
			BaseAgent* base = AgentManager::getInstance()->getClosestBase(pos);
			double dist = pos.getDistance(base->getUnit()->getTilePosition());
			if (dist <= 12)
			{
				//We have a base near this base location
				//Check if we have minerals available
				Unit* cMineral = hasMineralNear(pos);
				if (cMineral != NULL)
				{
					mineral = cMineral;
					bestDist = cDist;
				}
			}
		}
	}

	//We have no base with minerals, do nothing
	return mineral;
}

Unit* CoverMap::hasMineralNear(const TilePosition& pos) const
{
	for(set<Unit*>::iterator m = Broodwar->getMinerals().begin(); m != Broodwar->getMinerals().end(); m++)
	{
		if ((*m)->exists() && (*m)->getResources() > 0)
		{
			double dist = pos.getDistance((*m)->getTilePosition());
			if (dist <= 10)
			{
				return (*m);
			}
		}
	}
	return NULL;
}

bool CoverMap::suitableForDetector(const TilePosition& pos) const
{
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++)
	{
		BaseAgent* agent = agents[i];
		UnitType type = agent->getUnitType();
		if (agent->isAlive() && type.isDetector() && type.isBuilding())
		{
			double range = type.sightRange() * 1.6;
			double dist = agent->getUnit()->getPosition().getDistance(Position(pos));
			if (dist <= range)
			{
				return false;
			}
		}
	}
	return true;
}

void CoverMap::printGraphicDebugInfo()
{
	if (bats::config::debug::GRAPHICS_VERBOSITY == bats::config::debug::GraphicsVerbosity_Off ||
		bats::config::debug::modules::COVER_MAP == false)
	{
		return;
	}


	// High
	// Draw blocked and temporarily blocked squares
	bool drawBox;
	Color drawColor;
	Position min;
	Position max;
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_High) {
		for (int x = 0; x < mMapWidth; x++) {
			min.x() = x * TILE_SIZE;
			max.x() = min.x() + TILE_SIZE - 1;
			for (int y = 0; y < mMapHeight; y++) {
				drawBox = false;

				switch (mCoverMap[x][y]) {
				case TileState_TempBlocked:
					drawBox = true;
					drawColor = Colors::Orange;
					break;

				case TileState_Mineral:
					drawBox = true;
					drawColor = Colors::Cyan;
					break;

				case TileState_Gas:
					drawBox = true;
					drawColor = Colors::Green;
					break;

				case TileState_Blocked:
					drawBox = true;
					drawColor = Colors::Red;
					break;

				case TileState_ExpansionReserved:
					drawBox = true;
					drawColor = Colors::Yellow;
					break;

				// Test for units on the tile
				//case TileState_Buildable: {
				//	const vector<Unit*>& teamUnits = bats::UnitHelper::getTeamUnits();
				//	TilePosition currentPos(x,y);
				//	if (bats::UnitHelper::unitsInArea(teamUnits, currentPos, currentPos)) {
				//		drawBox = true;
				//		drawColor = Colors::Purple;
				//	}
				//	break;
				//}

				default:
					// Does nothing					
					break;
				}

				if (drawBox) {
					min.y() = y * TILE_SIZE;
					max.y() = min.y() + TILE_SIZE - 1;
					Broodwar->drawBoxMap(min.x(), min.y(), max.x(), max.y(), drawColor);
				}
			}
		}
	}
}
