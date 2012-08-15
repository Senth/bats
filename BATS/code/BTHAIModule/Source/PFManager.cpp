#include "PFManager.h"
#include "PFFunctions.h"
#include "UnitAgent.h"
#include "AgentManager.h"
#include "CoverMap.h"
#include "Profiler.h"
#include "BaseAgent.h"
#include "UnitAgent.h"
#include "BATSModule/include/Helper.h"
#include "BATSModule/include/Config.h"

using namespace BWAPI;
using namespace std;

bool PFManager::instanceFlag = false;
PFManager* PFManager::instance = NULL;

PFManager::PFManager()
{
	checkRange = 48;
	stepSize = 16;
	mapW = Broodwar->mapWidth() * 32;
	mapH = Broodwar->mapHeight() * 32;
}

PFManager::~PFManager()
{
	instanceFlag = false;
	instance = NULL;
}

PFManager* PFManager::getInstance()
{
	if (!instanceFlag)
	{
		instance = new PFManager();
		instanceFlag = true;
	}
	return instance;
}

void PFManager::computeAttackingUnitActions(BaseAgent* agent, TilePosition goal, bool defensive, bool forceMove)
{
	Unit* unit = agent->getUnit();

	// Stop if goal is invalid
	if (goal == TilePositions::Invalid) {
		unit->stop();
		return;
	}

	if (!defensive && !forceMove)
	{
		//if (agent->getUnit()->isStartingAttack()) return;
		if (agent->getUnit()->isAttacking()) return;
		if (agent->getUnit()->isSieged()) return;
	}
	if (agent->getUnit()->isLoaded()) return;


	//PF
	int unitX = unit->getPosition().x();
	int unitY = unit->getPosition().y();

	// Currently no need to check if force movement since it will not use potential fields
	//if (!forceMove) {
		// Checks if there are enemy units close, then use potential fields
		int unitSight = Broodwar->self()->sightRange(agent->getUnitType());
		bool enemyClose = false;
		const set<Player*>& players = Broodwar->enemies();
		set<Player*>::const_iterator playerIt = players.begin();
		while (!enemyClose && playerIt != players.end()) {
			const set<Unit*>& enemyUnits = (*playerIt)->getUnits();
			set<Unit*>::const_iterator unitIt = enemyUnits.begin();

			while (!enemyClose && unitIt != enemyUnits.end()) {
				// Enemy within sight range of the unit
				if (bats::isWithinRange(agent->getUnit()->getPosition(), (*unitIt)->getPosition(), unitSight)) {
					enemyClose = true;
				}
				// Our unit within the sight range of the enemy
				else if (bats::isWithinRange(agent->getUnit()->getPosition(), (*unitIt)->getPosition(), (*playerIt)->sightRange((*unitIt)->getType()))) {
					enemyClose = true;
				}
				// Enemy can target our unit (long range attacks)
				else if ((*unitIt)->isInWeaponRange(agent->getUnit())) {
					enemyClose = true;
				}
				// We can target enemy unit (long range attacks)
				else if (agent->getUnit()->isInWeaponRange(*unitIt)) {
					enemyClose = true;
				}

				++unitIt;
			}

			++playerIt;
		}


		// Unit in attacking state
		if (enemyClose) {

			float bestP = getAttackingUnitP(agent, unitX, unitY, defensive);
			float cP = 0;
	
			float startP = bestP;
			int bestX = -1;
			int bestY = -1;

			for (int cX = unitX - checkRange; cX <= unitX + checkRange; cX += stepSize)
			{
				for (int cY = unitY - checkRange; cY <= unitY + checkRange; cY += stepSize)
				{
					if (cX >= 0 && cY >= 0 && cX <= mapW && cY <= mapH)
					{
						cP = getAttackingUnitP(agent, cX, cY, defensive);
						if (forceMove) {
							cP += PFFunctions::getGoalP(Position(cX, cY), agent->getGoal());
						}

						if (cP > bestP)
						{
							bestP = cP;
							bestX = cX;
							bestY = cY;
						}
					}
				}
			}
	
			if (bestX >= 0 && bestY >= 0)
			{
				Position toMove(bestX, bestY);
				if (!defensive && !forceMove)
				{
					unit->attack(toMove);
				}
				else
				{
					unit->rightClick(toMove);
				}
				return; // RETURNING
			}
		}
		// Enemy not close, use regular path finding
		else if (goal != TilePositions::Invalid) {
			moveToGoal(agent, goal, defensive, forceMove);
		}
	//}
	//// Movement forced, use regular path finding
	//else {
	//	moveToGoal(agent, goal, defensive, forceMove);		
	//}
}

void PFManager::displayPF(const UnitAgent* agent)
{
	if (bats::config::debug::GRAPHICS_VERBOSITY == bats::config::debug::GraphicsVerbosity_Off ||
		bats::config::debug::modules::POTENTIAL_FIELDS == false)
	{
		return;
	}

	
	// High
	// Print potential fields if enemy is close
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_High) {
		int unitSight = Broodwar->self()->sightRange(agent->getUnitType());
		bool enemyClose = false;
		const set<Player*>& players = Broodwar->enemies();
		set<Player*>::const_iterator playerIt = players.begin();
		while (!enemyClose && playerIt != players.end()) {
			const set<Unit*>& enemyUnits = (*playerIt)->getUnits();
			set<Unit*>::const_iterator unitIt = enemyUnits.begin();

			while (!enemyClose && unitIt != enemyUnits.end()) {
				// Enemy within sight range of the unit
				if (bats::isWithinRange(agent->getUnit()->getPosition(), (*unitIt)->getPosition(), unitSight)) {
					enemyClose = true;
				}
				// Our unit within the sight range of the enemy
				else if (bats::isWithinRange(agent->getUnit()->getPosition(), (*unitIt)->getPosition(), (*playerIt)->sightRange((*unitIt)->getType()))) {
					enemyClose = true;
				}
				// Enemy can target our unit (long range attacks)
				else if ((*unitIt)->isInWeaponRange(const_cast<Unit*>(agent->getUnit()))) {
					enemyClose = true;
				}
				// We can target enemy unit (long range attacks)
				else if (agent->getUnit()->isInWeaponRange(*unitIt)) {
					enemyClose = true;
				}

				++unitIt;
			}

			++playerIt;
		}

		if (enemyClose) {
			const Unit* unit = agent->getUnit();

			//PF
			int tileX = unit->getTilePosition().x();
			int tileY = unit->getTilePosition().y();
			int range = 12;

			for (int cTileX = tileX - range; cTileX <= tileX + range; cTileX++)
			{
				for (int cTileY = tileY - range; cTileY <= tileY + range; cTileY++)
				{
					if (cTileX >= 0 && cTileY >= 0 && cTileX < Broodwar->mapWidth() && cTileY < Broodwar->mapHeight())
					{
						int cX = cTileX * 32 + 16;
						int cY = cTileY * 32 + 16;
				
						if (unit->hasPath(Position(cX, cY)))
						{
							float p = 0.0f;
							p += getAttackingUnitP(agent, cX, cY, agent->useDefensiveMode());
							//cP += PFFunctions::getGoalP(Position(cX,cY), goal);
							//cP += PFFunctions::getTrailP(agent, cX, cY);
							//cP += PFFunctions::getTerrainP(agent, cX, cY);
					
							//print box
							Broodwar->drawBoxMap(cTileX*32+3,cTileY*32+3,cTileX*32+26,cTileY*32+26,getColor(p),true);
						}
					}
				}
			}
		}
	}
}

Color PFManager::getColor(float p)
{
	if (p >= 0)
	{
		int v = (int)(p * 1.6);
		int halfV = (int)(p * 0.8);

		if (v > 255) v = 255;
		if (halfV > 255) halfV = 255;

		return Color(halfV, halfV, v);
	}
	else
	{
		p = -p;
		int v = (int)(p * 1.6);
		int halfV = (int)(p * 0.8);

		if (v > 255) v = 255;
		if (halfV > 175) halfV = 175; //255

		return Color(v, halfV, halfV);
	}
}

bool PFManager::moveToGoal(BaseAgent* agent, const TilePosition& goal, bool defensive, bool forceMove)
{
	if (goal == TilePositions::Invalid) return false;
	Unit* unit = agent->getUnit();

	if (!defensive && !forceMove) {
		if (unit->isStartingAttack() || unit->isAttacking())
		{
			return false;
		}
	}

	Position toReach = Position(goal);
	double distToReach = toReach.getDistance(unit->getPosition());

	int engageDist = unit->getType().groundWeapon().maxRange();
	if (agent->isOfType(UnitTypes::Terran_Medic))
	{
		engageDist = 6 * TILE_SIZE;
	}
	else if (engageDist <= 64)
	{
		engageDist = 64;
	}
	else if (engageDist >= 100)
	{
		engageDist = 100;
	}
	
	if (distToReach <= engageDist)
	{
		if (unit->isMoving()) unit->stop();
		return true;
	}

	
	// Don't attack when defensive
	if (defensive || forceMove) {
		return unit->move(toReach);
	} else {
		return unit->attack(toReach);
	}
}

float PFManager::getAttackingUnitP(const BaseAgent* agent, int cX, int cY, bool defensive)
{
	float p = 0;
	Position pos(cX, cY);

	//Enemy Units
	int eCnt = 0;
	float p_plus = 0;
	float p_minus = 0;
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();++i)
	{
		//Enemy seen
		float dist = PFFunctions::getDistance(pos, (*i));
		float ptmp = PFFunctions::calcAttackingUnitP(dist, agent->getUnit(), (*i), defensive);

		if (ptmp > 0)
		{
			if (ptmp > p_plus)
			{
				p_plus = ptmp;
			}
		}
		if (ptmp < 0)
		{
			p_minus += ptmp;
		}
	}
	if (p_minus < 0)
	{
		p = p_minus;
	}
	else
	{
		p = p_plus;
	}

	//No enemy units found, use pathfinding
	if (p == 0)
	{
		return p;
	}
	
	//Own Units
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); ++i)
	{
		if (agents.at(i)->isAlive())
		{
			float dist = PFFunctions::getDistance(pos, agents.at(i)->getUnit());
			float ptmp = PFFunctions::calcOwnUnitP(dist, agent->getUnit(), agents.at(i)->getUnit());

			p += ptmp;
		}
	}

	//Neutral Units
	for(set<Unit*>::const_iterator i=Broodwar->getNeutralUnits().begin();i!=Broodwar->getNeutralUnits().end();++i)
	{
		float dist = PFFunctions::getDistance(pos, (*i));
		float ptmp = PFFunctions::calcMineP(dist, agent->getUnit());

        p += ptmp;
	}

	//Allied units
	// Matteus Magnusson
	const set<Player*>& players = Broodwar->getPlayers();
	set<Player*>::const_iterator playerIt;
	for (playerIt = players.begin(); playerIt != players.end(); ++playerIt)
	{
		if (*playerIt != Broodwar->self() && (*playerIt)->isAlly(Broodwar->self()))
		{
			const set<Unit*>& units = (*playerIt)->getUnits();
			set<Unit*>::const_iterator unitIt;
			for (unitIt = units.begin(); unitIt != units.end(); ++unitIt)
			{
				float dist = PFFunctions::getDistance(pos, *unitIt);
				float ptmp = PFFunctions::calcOwnUnitP(dist, agent->getUnit(), *unitIt);

				p += ptmp;
			}
		}
	}

	//Broodwar->printf("[%d] o=%d", agent->getUnitID(), (int)p);

	//Terrain
	if (!Broodwar->isWalkable(cX/8, cY/8))
	{
		p -= 50;
	}

	//Broodwar->printf("[%d] t=%d", agent->getUnitID(), (int)p);

	return p;
}
