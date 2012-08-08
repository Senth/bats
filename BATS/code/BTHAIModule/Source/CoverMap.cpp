#include "CoverMap.h"
#include "AgentManager.h"
#include "BaseAgent.h"
#include "BatsModule/include/BuildPlanner.h"
#include "BatsModule/include/ExplorationManager.h"
#include "BatsModule/include/Config.h"
#include "Profiler.h"

using namespace BWAPI;
using namespace BWTA;
using namespace std;


bool CoverMap::instanceFlag = false;
CoverMap* CoverMap::instance = NULL;
bats::ExplorationManager* CoverMap::msExplorationManager = NULL;

CoverMap::CoverMap()
{
	msExplorationManager = bats::ExplorationManager::getInstance();

	w = Broodwar->mapWidth();
	h = Broodwar->mapHeight();
	range = 30;

	Unit* worker = findWorker();

	cover_map = new int*[w];
	for(int i = 0 ; i < w ; i++)
	{
		cover_map[i] = new int[h];

		//Fill from static map and Region connectability
		for (int j = 0; j < h; j++)
		{
			int ok = BUILDABLE;
			if (!Broodwar->isBuildable(i, j))
			{
				ok = BLOCKED;
			}

			cover_map[i][j] = ok;
		}
	}

	//Fill from current agents
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isBuilding())
		{
			Corners c = getCorners(agent->getUnit());
			fill(c);
		}
	}

	//Fill from minerals
	for(set<Unit*>::iterator m = Broodwar->getMinerals().begin(); m != Broodwar->getMinerals().end(); m++)
	{
		Corners c;
		c.x1 = (*m)->getTilePosition().x() - 1;
		c.y1 = (*m)->getTilePosition().y() - 1;
		c.x2 = (*m)->getTilePosition().x() + 2;
		c.y2 = (*m)->getTilePosition().y() + 1;
		fill(c);

		cover_map[c.x1+2][c.y1+2] = MINERAL;
	}

	//Fill from gas
	for(set<Unit*>::iterator m = Broodwar->getGeysers().begin(); m != Broodwar->getGeysers().end(); m++)
	{
		Corners c;
		c.x1 = (*m)->getTilePosition().x() - 2;
		c.y1 = (*m)->getTilePosition().y() - 2;
		c.x2 = (*m)->getTilePosition().x() + 5;
		c.y2 = (*m)->getTilePosition().y() + 3;
		fill(c);

		cover_map[c.x1+2][c.y1+2] = GAS;
	}

	//Fill from narrow chokepoints
	if (analyzed)
	{
		for(set<BWTA::Region*>::const_iterator i=getRegions().begin();i!=getRegions().end();i++)
		{
			for(set<Chokepoint*>::const_iterator c=(*i)->getChokepoints().begin();c!=(*i)->getChokepoints().end();c++)
			{
				if ((*c)->getWidth() <= 4 * 32)
				{
					TilePosition center = TilePosition((*c)->getCenter());
					Corners c;
					c.x1 = center.x() - 1;
					c.x2 = center.x() + 1;
					c.y1 = center.y() - 1;
					c.y2 = center.y() + 1;
					fill(c);
				}
			}
		}
	}

	mapData = MapDataReader();
	mapData.readMap();
}

CoverMap::~CoverMap()
{
	for(int i = 0 ; i < w ; i++)
	{
		delete[] cover_map[i];
	}
	delete[] cover_map;

	instanceFlag = false;
	instance = NULL;
	msExplorationManager = NULL;
}

CoverMap* CoverMap::getInstance()
{
	if (!instanceFlag)
	{
		instance = new CoverMap();
		instanceFlag = true;
	}
	return instance;
}


Unit* CoverMap::findWorker()
{
	BaseAgent* worker = AgentManager::getInstance()->getAgent(Broodwar->self()->getRace().getWorker());
	if (worker != NULL)
	{
		return worker->getUnit();
	}
	return NULL;
}

bool CoverMap::positionFree(TilePosition pos)
{
	if (cover_map[pos.x()][pos.y()] == BUILDABLE)
	{
		return true;
	}
	return false;
}

void CoverMap::blockPosition(TilePosition buildSpot)
{
	if (buildSpot != TilePositions::Invalid)
	{
		//Error check
		return;
	}
	cover_map[buildSpot.x()][buildSpot.y()] = BLOCKED;
}

bool CoverMap::canBuild(UnitType toBuild, TilePosition buildSpot)
{
	Corners c = getCorners(toBuild, buildSpot);

	//Step 1: Check covermap.
	for (int x = c.x1; x <= c.x2; x++)
	{
		for (int y = c.y1; y <= c.y2; y++)
		{
			if (x >= 0 && x < w && y >= 0 && y < h)
			{
				if (cover_map[x][y] != BUILDABLE)
				{
					//Cant build here.
					return false;
				}
			}
			else
			{
				//Out of bounds
				return false;
			}
		}
	}

	//Step 2: Check if path is available
	if (!msExplorationManager->canReach(Broodwar->self()->getStartLocation(), buildSpot))
	{
		return false;
	}

	//Step 3: Check canBuild
	Unit* worker = findWorker();
	if (worker == NULL)
	{
		//No worker available
		return false;
	}

	//Step 4: Check any units on tile
	if (AgentManager::getInstance()->unitsInArea(buildSpot, toBuild.tileWidth(), toBuild.tileHeight(), worker->getID()))
	{
		return false;
	}

	//Step 5: If Protoss, check PSI coverage
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

	//Step 6: If Zerg, check creep
	if (bats::BuildPlanner::isZerg())
	{
		if (UnitAgent::isOfType(toBuild, UnitTypes::Zerg_Hatchery))
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

	//Step 7: If detector, check if spot is already covered by a detector
	/*if (toBuild.isDetector())
	{
		if (!suitableForDetector(buildSpot))
		{
			return false;
		}
	}*/

	//All passed. It is possible to build here.
	return true;
}

TilePosition CoverMap::findBuildSpot(UnitType toBuild)
{
	//Refinery
	if (toBuild.isRefinery())
	{
		//Use refinery method
		return findRefineryBuildSpot(toBuild, Broodwar->self()->getStartLocation());
	}

	//If we find unpowered buildings, build a Pylon there
	if (BaseAgent::isOfType(toBuild, UnitTypes::Protoss_Pylon))
	{
		vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
		for (int i = 0; i < (int)agents.size(); i++)
		{
			BaseAgent* agent = agents.at(i);
			if (agent->isAlive())
			{
				Unit* cUnit = agent->getUnit();
				if (cUnit->isUnpowered())
				{
					//Broodwar->printf("Build pylon at unpowered building %s", cUnit->getType().getName().c_str());
					TilePosition spot = findBuildSpot(toBuild, cUnit->getTilePosition());
					return spot;
				}
			}
		}
	}

	if (BaseAgent::isOfType(toBuild, UnitTypes::Protoss_Pylon))
	{
		if (AgentManager::getInstance()->countNoUnits(UnitTypes::Protoss_Pylon) > 0)
		{
			TilePosition cp = findChokepoint();
			if (cp != TilePositions::Invalid)
			{
				if (!Broodwar->hasPower(cp, UnitTypes::Protoss_Cybernetics_Core))
				{
					if (AgentManager::getInstance()->noInProduction(UnitTypes::Protoss_Pylon) == 0)
					{
						TilePosition spot = findBuildSpot(toBuild, cp);
						return spot;
					}
				}
			}
		}
	}

	//Build near chokepoints: Bunker, Photon Cannon, Creep Colony
	if (BaseAgent::isOfType(toBuild, UnitTypes::Terran_Bunker) ||
		BaseAgent::isOfType(toBuild, UnitTypes::Protoss_Photon_Cannon) ||
		BaseAgent::isOfType(toBuild, UnitTypes::Zerg_Creep_Colony))
	{
		TilePosition cp = findChokepoint();
		if (cp != TilePositions::Invalid)
		{
			TilePosition spot = findBuildSpot(toBuild, cp);
			return spot;
		}
	}

	//Base buildings.
	if (toBuild.isResourceDepot())
	{
		TilePosition start = findExpansionSite();
		if (start != TilePositions::Invalid)
		{
			TilePosition spot = findBuildSpot(toBuild, start);
			return spot;
		}
		else
		{
			//No expansion site found.
			return TilePositions::Invalid;
		}
	}

	//General building. Search for spot around bases
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = (int)agents.size() - 1; i >= 0; i--)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive() && agent->getUnitType().isResourceDepot() && !baseUnderConstruction(agent))
		{
			TilePosition start = agent->getUnit()->getTilePosition();
			TilePosition bSpot = findBuildSpot(toBuild, start);
			if (bSpot!= TilePositions::Invalid)
			{
				//Spot found, return it.
				return bSpot;
			}
		}
	}

	return TilePositions::Invalid;
}

bool CoverMap::baseUnderConstruction(BaseAgent* base)
{
	if (bats::BuildPlanner::isTerran())
	{
		return base->getUnit()->isBeingConstructed();
	}
	if (bats::BuildPlanner::isProtoss())
	{
		return base->getUnit()->isBeingConstructed();
	}
	if (bats::BuildPlanner::isZerg())
	{
		if (base->isOfType(UnitTypes::Zerg_Hatchery))
		{
			return base->getUnit()->isBeingConstructed();
		}
	}
	return false;
}

TilePosition CoverMap::findSpotAtSide(UnitType toBuild, TilePosition start, TilePosition end)
{
	int dX = end.x() - start.x();
	if (dX != 0) dX = 1;
	int dY = end.y() - start.y();
	if (dY != 0) dY = 1;

	TilePosition cPos = start;
	bool done = false;
	while (!done) 
	{
		if (canBuildAt(toBuild, cPos)) return cPos;
		int cX = cPos.x() + dX;
		int cY = cPos.y() + dY;
		cPos = TilePosition(cX, cY);
		if (cPos.x() == end.x() && cPos.y() == end.y()) done = true;
	}

	return TilePositions::Invalid;
}

bool CoverMap::isOccupied(BWTA::Region* region) const
{
	BWTA::Polygon p = region->getPolygon();
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive() && (agent->isBuilding() && !agent->isOfType(UnitTypes::Protoss_Pylon)))
		{
			BWTA::Region* aRegion = getRegion(agents.at(i)->getUnit()->getTilePosition());
			Position c1 = region->getCenter();
			Position c2 = aRegion->getCenter();
			if (c2.x() == c1.x() && c2.y() == c1.y())
			{
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
	TilePosition ePos = msExplorationManager->getClosestSpottedBuilding(center).first;

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

bool CoverMap::isEdgeChokepoint(Chokepoint* choke) const
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

TilePosition CoverMap::findDefensePos(Chokepoint* choke) const
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
				if (msExplorationManager->canReach(basePos, cPos))
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

bool CoverMap::canBuildAt(UnitType toBuild, TilePosition pos)
{
	int maxW = w - toBuild.tileWidth() - 1;
	int maxH = h - toBuild.tileHeight() - 1;

	//Out of bounds check
	if (pos != TilePositions::Invalid && pos.x() < maxW && pos.y() >= 0 && pos.y() < maxH)
	{
		if (canBuild(toBuild, pos))
		{
			return true;
		}
	}
	return false;
}

TilePosition CoverMap::findBuildSpot(UnitType toBuild, TilePosition start)
{
	//Check start pos
	if (canBuildAt(toBuild, start)) return start;

	//Search outwards
	bool found = false;
	int cDiff = 1;
	TilePosition spot = TilePositions::Invalid;
	while (!found) 
	{
		//Top
		TilePosition s = TilePosition(start.x() - cDiff, start.y() - cDiff);
		TilePosition e = TilePosition(start.x() + cDiff, start.y() - cDiff);
		spot = findSpotAtSide(toBuild, s, e);
		if (spot!= TilePositions::Invalid && spot.y() != -1)
		{
			found = true;
			break;
		}

		//Bottom
		s = TilePosition(start.x() - cDiff, start.y() + cDiff);
		e = TilePosition(start.x() + cDiff, start.y() + cDiff);
		spot = findSpotAtSide(toBuild, s, e);
		if (spot!= TilePositions::Invalid && spot.y() != -1)
		{
			found = true;
			break;
		}

		//Left
		s = TilePosition(start.x() - cDiff, start.y() - cDiff);
		e = TilePosition(start.x() - cDiff, start.y() + cDiff);
		spot = findSpotAtSide(toBuild, s, e);
		if (spot!= TilePositions::Invalid && spot.y() != -1)
		{
			found = true;
			break;
		}

		//Right
		s = TilePosition(start.x() + cDiff, start.y() - cDiff);
		e = TilePosition(start.x() + cDiff, start.y() + cDiff);
		spot = findSpotAtSide(toBuild, s, e);
		if (spot!= TilePositions::Invalid && spot.y() != -1)
		{
			found = true;
			break;
		}

		cDiff++;
		if (cDiff > range) found = true;
	}
	
	return spot;
}

void CoverMap::addConstructedBuilding(Unit* unit)
{
	if (unit->getType().isAddon())
	{
		//Addons are handled by their main buildings
		return;
	}

	Corners c = getCorners(unit);
	fill(c);
}

void CoverMap::buildingDestroyed(Unit* unit)
{
	if (unit->getType().isAddon())
	{
		//Addons are handled by their main buildings
		return;
	}

	Corners c = getCorners(unit);
	clear(c);
}

void CoverMap::fill(Corners c)
{
	for (int x = c.x1; x <= c.x2; x++)
	{
		for (int y = c.y1; y <= c.y2; y++)
		{
			if (x >= 0 && x < w && y >= 0 && y < h)
			{
				if (cover_map[x][y] == BUILDABLE)
				{
					cover_map[x][y] = BLOCKED;
				}
				if (cover_map[x][y] == TEMPBLOCKED)
				{
					cover_map[x][y] = BLOCKED;
				}
			}
		}
	}
}

void CoverMap::fillTemp(UnitType toBuild, TilePosition buildSpot)
{
	Corners c = getCorners(toBuild, buildSpot);

	for (int x = c.x1; x <= c.x2; x++)
	{
		for (int y = c.y1; y <= c.y2; y++)
		{
			if (x >= 0 && x < w && y >= 0 && y < h)
			{
				if (cover_map[x][y] == BUILDABLE)
				{
					cover_map[x][y] = TEMPBLOCKED;
				}
			}
		}
	}
}

void CoverMap::clear(Corners c)
{
	for (int x = c.x1; x <= c.x2; x++)
	{
		for (int y = c.y1; y <= c.y2; y++)
		{
			if (x >= 0 && x < w && y >= 0 && y < h)
			{
				if (cover_map[x][y] == BLOCKED)
				{
					cover_map[x][y] = BUILDABLE;
				}
				if (cover_map[x][y] == TEMPBLOCKED)
				{
					cover_map[x][y] = BUILDABLE;
				}
			}
		}
	}
}

void CoverMap::clearTemp(UnitType toBuild, TilePosition buildSpot)
{
	if (buildSpot== TilePositions::Invalid)
	{
		return;
	}

	Corners c = getCorners(toBuild, buildSpot);

	for (int x = c.x1; x <= c.x2; x++)
	{
		for (int y = c.y1; y <= c.y2; y++)
		{
			if (x >= 0 && x < w && y >= 0 && y < h)
			{
				if (cover_map[x][y] == TEMPBLOCKED)
				{
					cover_map[x][y] = BUILDABLE;
				}
			}
		}
	}
}

Corners CoverMap::getCorners(Unit* unit)
{
	return getCorners(unit->getType(), unit->getTilePosition());
}

Corners CoverMap::getCorners(UnitType type, TilePosition center)
{
	int x1 = center.x();
	int y1 = center.y();
	int x2 = x1 + type.tileWidth() - 1;
	int y2 = y1 + type.tileHeight() - 1;

	int margin = 1;
	if (type.canProduce())
	{
		margin = 1;
	}

	if (BaseAgent::isOfType(type, UnitTypes::Terran_Supply_Depot))
	{
		margin = 0;
	}
	if (BaseAgent::isOfType(type, UnitTypes::Protoss_Pylon))
	{
		margin = 0;
	}

	x1 -= margin;
	x2 += margin;
	y1 -= margin;
	y2 += margin;

	//Special case: Terran Addon buildings
	//Add 2 extra spaces to the right to make space for the addons.
	if (BaseAgent::isOfType(type, UnitTypes::Terran_Factory) || BaseAgent::isOfType(type, UnitTypes::Terran_Starport) || BaseAgent::isOfType(type, UnitTypes::Terran_Command_Center) || BaseAgent::isOfType(type, UnitTypes::Terran_Science_Facility))
	{
		x2 += 2;
	}

	Corners c;
	c.x1 = x1;
	c.y1 = y1;
	c.x2 = x2;
	c.y2 = y2;

	return c;
}

TilePosition CoverMap::findRefineryBuildSpot(UnitType toBuild, TilePosition start)
{
	TilePosition buildSpot = findClosestGasWithoutRefinery(toBuild, start);
	if (buildSpot != TilePositions::Invalid)
	{
		BaseAgent* base = AgentManager::getInstance()->getClosestBase(buildSpot);
		if (base == NULL)
		{
			Broodwar->printf("No base found");
			return TilePosition(-1,-1);
		}
		else
		{
			double dist = buildSpot.getDistance(base->getUnit()->getTilePosition());
			if (dist >= 13) 
			{
				Broodwar->printf("Base too far away %d", (int)dist);
				return TilePosition(-1,-1);
			}
		}

	}
	return buildSpot;
}

TilePosition CoverMap::findClosestGasWithoutRefinery(UnitType toBuild, TilePosition start)
{
	TilePosition bestSpot = TilePosition(-1,-1);
	double bestDist = -1;
	TilePosition home = Broodwar->self()->getStartLocation();
	Unit* worker = findWorker();

	for(int i = 0 ; i < w ; i++)
	{
		for (int j = 0; j < h; j++)
		{
			if (cover_map[i][j] == GAS)
			{
				TilePosition cPos = TilePosition(i,j);

				bool ok = true;
				vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
				for (int i = 0; i < (int)agents.size(); i++)
				{
					Unit* unit = agents.at(i)->getUnit();
					if (unit->getType().isRefinery())
					{
						double dist = unit->getTilePosition().getDistance(cPos);
						if (dist <= 2)
						{
							ok = false;
						}
					}
				}
				if (ok)
				{
					if (msExplorationManager->canReach(home, cPos))
					{
						BaseAgent* agent = AgentManager::getInstance()->getClosestBase(cPos);
						double dist = agent->getUnit()->getTilePosition().getDistance(cPos);
						if (bestDist == -1 || dist < bestDist)
						{
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

TilePosition CoverMap::searchRefinerySpot()
{
	for(int i = 0 ; i < w ; i++)
	{
		for (int j = 0; j < h; j++)
		{
			if (cover_map[i][j] == GAS)
			{
				TilePosition cPos = TilePosition(i,j);

				bool found = false;
				vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
				for (int i = 0; i < (int)agents.size(); i++)
				{
					if (agents.at(i)->getUnitType().isRefinery())
				{
						double dist = agents.at(i)->getUnit()->getTilePosition().getDistance(cPos);
						TilePosition uPos = agents.at(i)->getUnit()->getTilePosition();
						if (dist <= 2)
				{
							found = true;
							break;
						}
					}
				}

				if (!found)
				{
					BaseAgent* agent = AgentManager::getInstance()->getClosestBase(cPos);
					if (agent != NULL)
					{
						TilePosition bPos = agent->getUnit()->getTilePosition();
						double dist = bPos.getDistance(cPos);

						if (dist < 15)
						{
							if (msExplorationManager->canReach(bPos, cPos))
							{
								return cPos;
							}			
						}
					}
				}
			}
		}
	}

	return TilePositions::Invalid;
}

TilePosition CoverMap::findExpansionSite()
{
	UnitType baseType = Broodwar->self()->getRace().getCenter();
	double bestDist = 100000;
	TilePosition bestPos = TilePositions::Invalid;
	
	//Iterate through all base locations
	for(set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin(); i!= BWTA::getBaseLocations().end(); i++)
	{
		TilePosition pos = (*i)->getTilePosition();
		bool taken = false;
		
		//Check if own buildings are close
		vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
		int noBases = 0;
		for (int i = 0; i < (int)agents.size(); i++)
		{
			BaseAgent* agent = agents.at(i);
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
		int eCnt = msExplorationManager->countSpottedBuildingsWithinRange(pos, 20);
		if (eCnt > 0)
		{
			taken = true;
		}

		//Not taken, calculate ground distance
		if (!taken)
		{
			if (msExplorationManager->canReach(Broodwar->self()->getStartLocation(), pos))
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

Unit* CoverMap::findClosestMineral(TilePosition workerPos)
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

Unit* CoverMap::hasMineralNear(TilePosition pos)
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

bool CoverMap::suitableForDetector(TilePosition pos)
{
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
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
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_High)
	{
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				if (cover_map[x][y] == TEMPBLOCKED)
				{
					Broodwar->drawBoxMap(x*32,y*32,x*32+31,y*32+31,Colors::Green);
				}
				if (cover_map[x][y] == BLOCKED)
				{
					Broodwar->drawBoxMap(x*32,y*32,x*32+31,y*32+31,Colors::Red);
				}
			}
		}
	}
}
