#include "UnitAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "TargetingAgent.h"
#include "SpottedObject.h"
#include "BATSModule/include/UnitHelper.h"
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
	if (bats::config::debug::GRAPHICS_VERBOSITY == bats::config::debug::GraphicsVerbosity_Off) {
		return;
	}

	if (!isAlive()) return;
	if (unit->isLoaded()) return;
	if (unit->isBeingConstructed()) return;
	if (!unit->isCompleted()) return;
	

	msPfManager->displayPF(this);
	printGraphicDebugSelectedUnits();

	if (bats::config::debug::modules::AGENT_UNIT) {
		// Medium
		if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Medium) {
			const Position& unitPos = unit->getPosition();
			const TilePosition& unitTilePos = unit->getTilePosition();

			if (goal != TilePositions::Invalid) {
				if (unit->isMoving())
				{
					Position b = Position(goal);
					Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Teal);
				}
				if(unit->isIdle()){
					Position b = Position(goal);		
					Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Teal);
				}
				if (!unit->isIdle())
				{
					Unit* targ = unit->getOrderTarget();
					if (targ != NULL && targ->exists())
					{
						Position b = Position(targ->getPosition());

						if (targ->getPlayer()->isEnemy(Broodwar->self()))
						{
							Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Red);
						}
						else
						{
							Broodwar->drawLineMap(unitPos.x(),unitPos.y(),b.x(),b.y(),Colors::Green);
						}
					}
				}
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
		}
	}
}

string UnitAgent::getDebugString() const {
	stringstream ss;

	// Medium - state
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Medium) {
		const Position& unitPos = unit->getPosition();
		const TilePosition& unitTilePos = unit->getTilePosition();
		if (goal != TilePositions::Invalid) {

			string state = "Unknown";

			if (unit->isMoving()) {
				state = "Move";
			}
			else if(unit->isIdle()){
				state = "Idle";
			}
		
			if (!unit->isIdle()) {
				Unit* target = unit->getOrderTarget();
				if (target != NULL && target->exists()) {
					if (target->getPlayer()->isEnemy(Broodwar->self())) {
						state = "Attack";
					}
					else {
						state = "Support";
					}
				}
			}

			state += ": ";

			ss << setw(bats::config::debug::GRAPHICS_COLUMN_WIDTH) << state << goal << "\n";
		}
	}

	return BaseAgent::getDebugString() + ss.str();
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

int UnitAgent::enemyUnitsWithinRange(int maxRange) const {
	int maxRangeSquared = maxRange * maxRange;
	int cEnemies = 0;
	const vector<Unit*>& enemyUnits = bats::UnitHelper::getEnemyUnits();

	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		if (enemyUnits[i]->exists()) {
			if (bats::isWithinRange(getUnit()->getPosition(), enemyUnits[i]->getPosition(), maxRangeSquared, true)) {
				++cEnemies;
			}
		}
	}

	return cEnemies;
}

int UnitAgent::enemyGroundUnitsWithinRange(int maxRange) const {
	if (maxRange < 0) {
		return 0;
	}

	int maxRangeSquared = maxRange * maxRange;
	int cEnemies = 0;
	const vector<Unit*>& enemyUnits = bats::UnitHelper::getEnemyUnits();

	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		if (!enemyUnits[i]->getType().isFlyer()) {
			if (bats::isWithinRange(getUnit()->getPosition(), enemyUnits[i]->getPosition(), maxRangeSquared, true)) {
				++cEnemies;
			}
		}
	}

	return cEnemies;
}

int UnitAgent::enemySiegedTanksWithinRange() const {
	int maxRange = 12 * 32 + 16;
	int maxRangeSquared = maxRange * maxRange;
	int cEnemies = 0;

	const vector<Unit*>& enemyUnits = bats::UnitHelper::getEnemyUnits();

	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		if (enemyUnits[i]->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode) {
			if (bats::isWithinRange(getUnit()->getPosition(), enemyUnits[i]->getPosition(), maxRangeSquared, true)) {
				++cEnemies;
			}
		}
	}

	return cEnemies;
}

int UnitAgent::enemyGroundAttackingUnitsWithinRange(int maxRange) const {
	if (maxRange < 0) {
		return 0;
	}

	int maxRangeSquared = maxRange * maxRange;
	int cEnemies = 0;
	const vector<Unit*>& enemyUnits = bats::UnitHelper::getEnemyUnits();

	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		if (!enemyUnits[i]->getType().isFlyer() && canAttack(enemyUnits[i]->getType(), getUnitType())) {
			if (bats::isWithinRange(getUnit()->getPosition(), enemyUnits[i]->getPosition(), maxRangeSquared, true)) {
				++cEnemies;
			}
		}
	}

	return cEnemies;
}

int UnitAgent::enemyAirUnitsWithinRange(int maxRange) const {
	if (maxRange < 0) {
		return 0;
	}

	int maxRangeSquared = maxRange * maxRange;
	int cEnemies = 0;
	const vector<BWAPI::Unit*>& enemyUnits = bats::UnitHelper::getEnemyUnits();

	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		if (enemyUnits[i]->getType().isFlyer() || (enemyUnits[i]->getType().isFlyingBuilding() && enemyUnits[i]->isLifted())) {
			if (bats::isWithinRange(getUnit()->getPosition(), enemyUnits[i]->getPosition(), maxRangeSquared, true)) {
				++cEnemies;
			}
		}
	}

	return cEnemies;
}

int UnitAgent::enemyAirToGroundUnitsWithinRange(int maxRange) const {
	if (maxRange < 0) {
		return 0;
	}

	int maxRangeSquared = maxRange * maxRange;
	int cEnemies = 0;
	const vector<BWAPI::Unit*>& enemyUnits = bats::UnitHelper::getEnemyUnits();

	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		if (enemyUnits[i]->getType().isFlyer()) {
			if (enemyUnits[i]->getType().groundWeapon().targetsGround() ||
				enemyUnits[i]->getType().airWeapon().targetsGround())
			{
				if (bats::isWithinRange(getUnit()->getPosition(), enemyUnits[i]->getPosition(), maxRangeSquared, true)) {
					++cEnemies;
				}
			}
		}
	}

	return cEnemies;
}

int UnitAgent::enemyAirAttackingUnitsWithinRange(int maxRange) const {
	if (maxRange < 0) {
		return 0;
	}

	int maxRangeSquared = maxRange * maxRange;
	int cEnemies = 0;
	const vector<BWAPI::Unit*>& enemyUnits = bats::UnitHelper::getEnemyUnits();

	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		if (enemyUnits[i]->getType().isFlyer() && canAttack(enemyUnits[i]->getType(), getUnitType())) {
			if (bats::isWithinRange(getUnit()->getPosition(), enemyUnits[i]->getPosition(), maxRangeSquared, true)) {
				++cEnemies;
			}
		}
	}

	return cEnemies;
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
	return enemyGroundAttackingUnitsWithinRange(getGroundRange()) + enemyAirAttackingUnitsWithinRange(getAirRange());
}

int UnitAgent::enemyAttackingUnitsWithinRange(int maxRange) const
{
	return enemyGroundAttackingUnitsWithinRange(maxRange) + enemyAirAttackingUnitsWithinRange(maxRange);
}

int UnitAgent::enemyAttackingUnitsWithinRange(const UnitType& type) const
{
	return enemyGroundAttackingUnitsWithinRange(getGroundRange(type)) + enemyAirAttackingUnitsWithinRange(getAirRange(type));
}

Unit* UnitAgent::getClosestOrganicEnemy(int maxRange)
{
	/// @todo does not work, use bats::getEnemyUnits() instead
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
	/// @todo does not work, use bats::getEnemyUnits() instead
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
	/// @todo does not work, use bats::getEnemyUnits() instead
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
	/// @todo does not work, use bats::getEnemyUnits() instead
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
	/// @todo include allied units
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

int UnitAgent::getGroundRange() const
{
	return getGroundRange(unit->getType());
}

int UnitAgent::getGroundRange(const UnitType& type)
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

int UnitAgent::getAirRange(const UnitType& type)
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

bool UnitAgent::canAttack(const UnitType& attacker, const UnitType& target)
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
					int eCnt = enemyAttackingUnitsWithinRange(12 * 32);
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