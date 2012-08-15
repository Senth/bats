#include "UnitAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "TargetingAgent.h"
#include "SpottedObject.h"
#include "BATSModule/include/Helper.h"
#include "BATSModule/include/SquadManager.h"
#include "BATSModule/include/Squad.h"
#include "BATSModule/include/Config.h"
#include "Utilities/Logger.h"
#include <sstream>
#include <iomanip>

using namespace BWAPI;
using namespace std;
using bats::operator<<;

bats::SquadManager* UnitAgent::msSquadManager = NULL;
PFManager* UnitAgent::msPfManager = NULL;

UnitAgent::UnitAgent(Unit* mUnit) : BaseAgent(mUnit)
{
	agentType = "UnitAgent";

	if (msSquadManager == NULL) {
		msSquadManager = bats::SquadManager::getInstance();
		msPfManager = PFManager::getInstance();
	}
}

void UnitAgent::printGraphicDebugInfo() const
{
	if (bats::config::debug::GRAPHICS_VERBOSITY == bats::config::debug::GraphicsVerbosity_Off ||
		bats::config::debug::modules::AGENT_UNIT == false)
	{
		return;
	}

	if (!isAlive()) return;
	if (unit->isLoaded()) return;
	if (unit->isBeingConstructed()) return;
	if (!unit->isCompleted()) return;
	

	msPfManager->displayPF(this);


	// Medium
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Medium) {
		const Position& unitPos = unit->getPosition();
		const TilePosition& unitTilePos = unit->getTilePosition();

		stringstream ss;
		ss << bats::TextColors::LIGHT_GREY <<
			setw(bats::config::debug::GRAPHICS_COLUMN_WIDTH) << "Id: " << getUnitID() << "\n";

		// High
		if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_High) {
			ss << setw(bats::config::debug::GRAPHICS_COLUMN_WIDTH) << "Pos: " << unitTilePos << "\n";
		}

		if (goal != TilePositions::Invalid) {

			string state = "Unknown";

			if (unit->isMoving())
			{
				Position b = Position(goal);
				Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Teal);

				state = "Move";
			}
			if(unit->isIdle()){
				Position b = Position(goal);		
				Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Teal);
				state = "Idle";
			}
			if (!unit->isIdle())
			{
				Unit* targ = unit->getOrderTarget();
				if (targ != NULL)
				{
					Position b = Position(targ->getPosition());

					if (targ->getPlayer()->isEnemy(Broodwar->self()))
					{
						if (targ->exists())
						{
							Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Red);
							state = "Attack";
						}
					}
					else
					{
						if (targ->exists())
						{
							Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Green);
							state = "Support";
						}
					}
				}
			}

			state += ": ";

			ss << setw(bats::config::debug::GRAPHICS_COLUMN_WIDTH) << state << goal << "\n";
		}

		if (unit->isBeingHealed())
		{
			Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 32, Colors::White);
		}

		if (unit->getType().isDetector())
		{
			double range = unit->getType().sightRange();
			Broodwar->drawCircleMap(unitPos.x(),unitPos.y(),(int)range, Colors::Red);
		}

		// Draw info text
		Broodwar->drawTextMap(unitPos.x(), unitPos.y()-10, "%s", ss.str().c_str());
	}
}

void UnitAgent::computeActions()
{
	// Does nothing
}

void UnitAgent::computeKitingActions()
{
	int range = (int)(unit->getType().groundWeapon().maxRange() - 5);
	if (unit->getType().isFlyer()) range = (int)(unit->getType().airWeapon().maxRange() - 5);
	
	int eCnt = enemyAttackingUnitsWithinRange(range);
	if (eCnt > 0)
	{

		//If Vulture, drop some mines
		if (isOfType(UnitTypes::Terran_Vulture))
		{
			if (Broodwar->self()->hasResearched(TechTypes::Spider_Mines))
			{
				if (unit->getSpiderMineCount() > 0)
				{
					unit->useTech(TechTypes::Spider_Mines, unit->getPosition());
					return;
				}
			}
		}

		unit->rightClick(Position(Broodwar->self()->getStartLocation()));
		return;
	}
	else
	{
		msPfManager->computeAttackingUnitActions(this, goal, false, false);
	}
}

bool UnitAgent::findAndTryAttack()
{
	bool avoidingEnemies = false; 
	bool attackingEnemy = false;

	// Check unit's squad state
	if (getSquadId().isValid()) {
		bats::SquadPtr squad = msSquadManager->getSquad(getSquadId());
		if (NULL != squad && squad->isAvoidingEnemies()) {
			avoidingEnemies = true;
		}
	}

	if (!avoidingEnemies) {
		Unit* pTargetUnit = TargetingAgent::findTarget(this);
		if (NULL != pTargetUnit) {
			attackingEnemy = unit->attack(pTargetUnit);
		}
	}

	return attackingEnemy;
}

void UnitAgent::computeMoveAction()
{
	bool defensive = false;
	bool forceMove = false;

	// Check unit's squad state
	if (getSquadId().isValid()) {
		bats::SquadPtr squad = msSquadManager->getSquad(getSquadId());
		if (NULL != squad && squad->isAvoidingEnemies()) {
			defensive = true;
			forceMove = true;
		}
	}

	computeMoveAction(defensive, forceMove);
}

void UnitAgent::computeMoveAction(bool defensive, bool forceMove)
{
	msPfManager->computeAttackingUnitActions(this, goal, defensive, forceMove);
}

int UnitAgent::enemyUnitsWithinRange(int maxRange) const
{
	int eCnt = 0;
	int j = 0;
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		//Enemy seen
		if ((*i)->exists())
		{
			double dist = unit->getDistance((*i));
			if (dist <= maxRange)
			{
				eCnt++;
			}
		}

		j++;
	}

	return eCnt;
}

int UnitAgent::enemyGroundUnitsWithinRange(int maxRange) const
{
	if (maxRange < 0)
	{
		return 0;
	}

	int eCnt = 0;
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		//Enemy seen
		if (!((*i)->getType().isFlyer()))
		{
			if ((*i)->exists())
			{
				double dist = unit->getDistance((*i));
				if (dist <= maxRange)
				{
					eCnt++;
				}
			}
		}
		
	}

	return eCnt;
}

int UnitAgent::enemySiegedTanksWithinRange(TilePosition center) const
{
	int maxRange = 12 * 32 + 16;
	int eCnt = 0;
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			if ((*i)->getType().getID() == UnitTypes::Terran_Siege_Tank_Siege_Mode.getID())
			{
				double dist = (*i)->getDistance(Position(center));
				if (dist <= maxRange)
				{
					eCnt++;
				}
			}
		}
	}

	return eCnt;
}

int UnitAgent::enemyGroundAttackingUnitsWithinRange(TilePosition center, int maxRange) const
{
	if (maxRange < 0)
	{
		return 0;
	}

	int eCnt = 0;
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		//Enemy seen
		if ((*i)->exists())
		{
			if (!((*i)->getType().isFlyer() || (*i)->getType().isFlyingBuilding()))
			{
				if (canAttack((*i)->getType(), unit->getType()))
				{
					double dist = (*i)->getDistance(Position(center));
					if (dist <= maxRange)
				{
						eCnt++;
					}
				}
			}
		}
	}

	return eCnt;
}

int UnitAgent::enemyAirUnitsWithinRange(int maxRange) const
{
	if (maxRange < 0)
	{
		return 0;
	}

	int eCnt = 0;
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		//Enemy seen
		if ((*i)->exists())
		{
			if ((*i)->getType().isFlyer() || (*i)->getType().isFlyingBuilding())
			{
				double dist = unit->getDistance((*i));
				if (dist <= maxRange)
				{
					eCnt++;
				}
			}
		}	
	}

	return eCnt;
}

int UnitAgent::enemyAirToGroundUnitsWithinRange(int maxRange) const
{
	if (maxRange < 0)
	{
		return 0;
	}

	int eCnt = 0;
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		//Enemy seen
		if ((*i)->exists())
		{
			UnitType type = (*i)->getType();
			if (type.groundWeapon().targetsGround() || type.airWeapon().targetsGround())
			{
				if ((*i)->getType().isFlyer() || (*i)->getType().isFlyingBuilding())
				{
					double dist = unit->getDistance((*i));
					if (dist <= maxRange)
					{
						eCnt++;
					}
				}
			}
		}	
	}

	return eCnt;
}

int UnitAgent::enemyAirAttackingUnitsWithinRange(TilePosition center, int maxRange) const
{
	if (maxRange < 0)
	{
		return 0;
	}

	int eCnt = 0;
	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		//Enemy seen
		if ((*i)->exists())
		{
			if ((*i)->getType().isFlyer() || (*i)->getType().isFlyingBuilding())
			{
				if (canAttack((*i)->getType(), unit->getType()))
				{
					double dist = (*i)->getDistance(Position(center));
					if (dist <= maxRange)
				{
						eCnt++;
					}
				}
			}
		}
		
	}

	return eCnt;
}

bool UnitAgent::useDefensiveMode() const
{
	if (isWeaponCooldown())
	{
		if (enemyAttackingUnitsWithinRange() > 0)
		{
			return true;
		}
	}
	return false;
}

int UnitAgent::enemyAttackingUnitsWithinRange() const
{
	return enemyGroundAttackingUnitsWithinRange(unit->getTilePosition(), getGroundRange()) + enemyAirAttackingUnitsWithinRange(unit->getTilePosition(), getAirRange());
}

int UnitAgent::enemyAttackingUnitsWithinRange(int maxRange, TilePosition center) const
{
	return enemyGroundAttackingUnitsWithinRange(center, maxRange) + enemyAirAttackingUnitsWithinRange(center, maxRange);
}

int UnitAgent::enemyAttackingUnitsWithinRange(UnitType type) const
{
	return enemyGroundAttackingUnitsWithinRange(unit->getTilePosition(), getGroundRange(type)) + enemyAirAttackingUnitsWithinRange(unit->getTilePosition(), getAirRange(type));
}

Unit* UnitAgent::getClosestOrganicEnemy(int maxRange)
{
	Unit* enemy = NULL;
	double bestDist = -1;

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			if ((*i)->getType().isOrganic() && !(*i)->getType().isBuilding())
			{
				double cDist = unit->getDistance((*i));
				if (bestDist < 0 || cDist < bestDist)
				{
					bestDist = cDist;
					enemy = (*i);
				}
			}
		}
	}

	return enemy;
}

Unit* UnitAgent::getClosestShieldedEnemy(int maxRange)
{
	Unit* enemy = NULL;
	double bestDist = -1;

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			if ((*i)->getShields() > 0)
			{
				double cDist = unit->getDistance((*i));
				if (bestDist < 0 || cDist < bestDist)
				{
					bestDist = cDist;
					enemy = (*i);
				}
			}
		}
	}

	return enemy;
}

Unit* UnitAgent::getClosestEnemyTurret(int maxRange)
{
	Unit* enemy = NULL;
	double bestDist = -1;

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			UnitType type = (*i)->getType();
			if (type.isBuilding() && type.canAttack())
			{
				double cDist = unit->getDistance((*i));
				if (bestDist < 0 || cDist < bestDist)
				{
					bestDist = cDist;
					enemy = (*i);
				}
			}
		}
	}

	return enemy;
}

Unit* UnitAgent::getClosestEnemyAirDefense(int maxRange)
{
	Unit* enemy = NULL;
	double bestDist = 100000;

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			UnitType type = (*i)->getType();
			
			bool canAttackAir = false;
			if (type.isBuilding())
			{
				if (type.groundWeapon().targetsAir()) canAttackAir = true;
				if (type.airWeapon().targetsAir()) canAttackAir = true;
			}

			if (canAttackAir)
			{
				double cDist = unit->getDistance((*i));
				if (cDist < bestDist)
				{
					bestDist = cDist;
					enemy = (*i);
				}
			}
		}
	}

	if (bestDist >= 0 && bestDist <= maxRange)
	{
		return enemy;
	}
	else
	{
		return NULL;
	}
}

int UnitAgent::friendlyUnitsWithinRange() const
{
	return friendlyUnitsWithinRange(192);
}

int UnitAgent::friendlyUnitsWithinRange(int maxRange) const
{
	int fCnt = 0;
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isUnit() && !agent->isOfType(UnitTypes::Terran_Medic))
		{
			double dist = unit->getDistance(agent->getUnit());
			if (dist <= maxRange)
			{
				fCnt++;
			}
		}
	}
	return fCnt;
}

int UnitAgent::friendlyUnitsWithinRange(TilePosition tilePos, int maxRange) const
{
	int fCnt = 0;
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
		for (int i = 0; i < (int)agents.size(); i++)
		{
		BaseAgent* agent = agents.at(i);
		if (agent->isUnit() && !agent->isOfType(UnitTypes::Terran_Medic))
		{
			double dist = agent->getUnit()->getDistance(Position(tilePos));
			if (dist <= maxRange)
			{
				fCnt++;
			}
		}
	}
	return fCnt;
}

int UnitAgent::getGroundRange() const
{
	return getGroundRange(unit->getType());
}

int UnitAgent::getGroundRange(UnitType type)
{
	WeaponType wep1 = type.groundWeapon();
	WeaponType wep2 = type.airWeapon();

	int maxRange = -1;
	if (wep1.targetsGround())
	{
		maxRange = wep1.maxRange();
	}
	if (wep2.targetsGround())
	{
		if (wep2.maxRange() > maxRange)
		{
			maxRange = wep2.maxRange();
		}
	}
	
	return maxRange;
}

int UnitAgent::getAirRange() const
{
	return getAirRange(unit->getType());
}

int UnitAgent::getAirRange(UnitType type)
{
	WeaponType wep1 = type.groundWeapon();
	WeaponType wep2 = type.airWeapon();

	int maxRange = -1;
	if (wep1.targetsAir())
	{
		maxRange = wep1.maxRange();
	}
	if (wep2.targetsAir())
	{
		if (wep2.maxRange() > maxRange)
		{
			maxRange = wep2.maxRange();
		}
	}
	
	return maxRange;
}

bool UnitAgent::canAttack(UnitType attacker, UnitType target)
{
	if (!attacker.canAttack())
	{
		return false;
	}

	if (target.isFlyer() || target.isFlyingBuilding())
	{
		if (getAirRange(attacker) >= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (getGroundRange(attacker) >= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool UnitAgent::isAir() const
{
	return unit->getType().isFlyer();
}

bool UnitAgent::isGround() const
{
	return !unit->getType().isFlyer();
}

bool UnitAgent::isTransport() const 
{
	UnitType unitType = unit->getType();
	bool isTransportType = unitType == UnitTypes::Terran_Dropship ||
		unitType == UnitTypes::Protoss_Shuttle;

	// Special case for Zerg overlord, they need to have the upgrade
	if (!isTransportType) {
		if (unitType == UnitTypes::Zerg_Overlord && unit->upgrade(UpgradeTypes::Ventral_Sacs)) {
			isTransportType = true;
		} else {
			isTransportType = false;
		}
	}
	return isTransportType;
}

bool UnitAgent::chargeShields()
{
	int cShields = unit->getShields();
	int maxShields = unit->getType().maxShields();

	if (cShields < maxShields)
	{
		//Shields are damaged
		BaseAgent* charger = AgentManager::getInstance()->getClosestAgent(unit->getTilePosition(), UnitTypes::Protoss_Shield_Battery);
		if (charger != NULL)
		{
			//Charger has energy
			if (charger->getUnit()->getEnergy() > 0)
			{
				double dist = charger->getUnit()->getTilePosition().getDistance(unit->getTilePosition());
				if (dist <= 15)
				{
					//We have charger nearby. Check if we have enemies around
					int eCnt = enemyAttackingUnitsWithinRange(12 * 32, unit->getTilePosition());
					if (eCnt == 0)
				{
						unit->rightClick(charger->getUnit());
						return true;
					}
				}
			}
		}
	}
	return false;
}

void UnitAgent::resetToDefaultBehavior() {
	/// @todo
}