#include "BaseAgent.h"
#include "AgentManager.h"
#include "ResourceManager.h"
#include "BATSModule/include/ExplorationManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "BatsModule/include/Config.h"
#include "BatsModule/include/Helper.h"
#include "BatsModule/include/UnitHelper.h"
#include "BatsModule/include/SquadDefs.h"
#include "BatsModule/include/DefenseManager.h"
#include "Utilities/Logger.h"
#include <BWAPI/Unit.h>
#include <sstream>
#include <iomanip>

using namespace BWAPI;
using namespace std;

bats::DefenseManager* BaseAgent::msDefenseManager = NULL;

BaseAgent::BaseAgent(Unit* mUnit)
{
	unit = mUnit;
	unitID = unit->getID();
	type = unit->getType();
	alive = true;
	lastActionFrame = 0;
	goal = TilePositions::Invalid;
	agentType = "BaseAgent";
	
	mCreationFrame = Broodwar->getFrameCount();

	if (msDefenseManager == NULL) {
		msDefenseManager = bats::DefenseManager::getInstance();
	}
}

BaseAgent::~BaseAgent()
{
	
}

void BaseAgent::setSquadId(bats::SquadId squadId) {
	this->mSquadId = squadId;
}

const bats::SquadId& BaseAgent::getSquadId() const {
	return mSquadId;
}

int BaseAgent::getCreationFrame() const {
	return mCreationFrame;
}

const string& BaseAgent::getTypeName() const
{
	return agentType;
}

bool BaseAgent::isBeingBuilt() const {
	return !unit->isCompleted();
}

bool BaseAgent::isCompleted() const {
	return unit->isCompleted();
}

int BaseAgent::getUnitID() const
{
	return unitID;
}

const UnitType& BaseAgent::getUnitType() const
{
	return type;
}

Unit* BaseAgent::getUnit()
{
	return unit;
}

const Unit* BaseAgent::getUnit() const
{
	return unit;
}

bool BaseAgent::matches(const Unit *mUnit) const
{
	if (isAlive())
	{
		if (mUnit->getID() == unitID)
		{
			return true;
		}
	}
	return false;
}

bool BaseAgent::isOfType(const UnitType& type) const
{
	if (unit->getType() == type)
	{
		return true;
	}
	return false;
}

bool BaseAgent::isOfType(const Unit* mUnit, const UnitType& type)
{
	if (mUnit->getType() == type)
	{
		return true;
	}
	return false;
}

bool BaseAgent::canBuild(const UnitType& type) const
{
	//1. Check if building is being constructed
	if (unit->isBeingConstructed())
	{
		return false;
	}

	//2. Check if we have enough resources
	if (!ResourceManager::getInstance()->hasResources(type))
	{
		return false;
	}

	//3. Check canMake
	if (!Broodwar->canMake(unit, type))
	{
		return false;
	}

	//4. All is clear.
	return true;
}

bool BaseAgent::isBuilding() const
{
	if (unit->getType().isBuilding())
	{
		return true;
	}
	return false;
}

bool BaseAgent::isWorker() const
{
	if (unit->getType().isWorker())
	{
		return true;
	}
	return false;
}

bool BaseAgent::isFreeWorker() const
{
	if (unit->getType().isWorker())
	{
		if (unit->isIdle() || (unit->isGatheringMinerals() && !unit->isCarryingMinerals()))
		{
			if (getSquadId().isInvalid())
			{
				return true;
			}
		}
	}
	return false;
}

bool BaseAgent::isUnit() const
{
	if (unit->getType().isBuilding() || unit->getType().isWorker() || unit->getType().isAddon())
	{
		return false;
	}
	return true;
}

bool BaseAgent::isUnderAttack() const
{
	bool attack = false;
	
	if (unit->getShields() < unit->getType().maxShields()) attack = true;
	if (unit->getHitPoints() < unit->getType().maxHitPoints()) attack = true;

	if (attack)
	{
		return unit->isUnderAttack();
	}
	return false;
}

void BaseAgent::destroyed()
{
	alive = false;
}

bool BaseAgent::isAlive() const
{
	return alive;
}

bool BaseAgent::isDamaged() const
{
	if (unit->getHitPoints() < unit->getType().maxHitPoints())
	{
		return true;
	}
	return false;
}

bool BaseAgent::isDetectorWithinRange(int range) const {
	int rangeSquared = range * range;
	const vector<Unit*>& enemyUnits = bats::UnitHelper::getEnemyUnits();

	for (size_t i = 0; i < enemyUnits.size(); ++i) {
		if (enemyUnits[i]->getType().isDetector()) {
			if (bats::isWithinRange(unit->getPosition(), enemyUnits[i]->getPosition(), rangeSquared, true)) {
				return true;
			}
		}
	}

	return false;
}

bool BaseAgent::doScannerSweep(const TilePosition& pos)
{
	if (!bats::BuildPlanner::isTerran())
	{
		return false;
	}

	if (!type.getID() == UnitTypes::Terran_Comsat_Station.getID())
	{
		return false;
	}

	if (unit->getEnergy() >= 50)
	{
		if (unit->useTech(TechTypes::Scanner_Sweep, Position(pos)))
		{
			Broodwar->printf("SCAN CLOAKED ENEMY");
			return true;
		}
	}

	return false;
}

bool BaseAgent::doEnsnare(const TilePosition& pos)
{
	if (!bats::BuildPlanner::isZerg())
	{
		return false;
	}
	if (!Broodwar->self()->hasResearched(TechTypes::Ensnare))
	{
		return false;
	}

	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive() && agent->isOfType(UnitTypes::Zerg_Queen))
		{
			if (agent->getUnit()->getEnergy() >= 75)
			{
				agent->getUnit()->useTech(TechTypes::Ensnare, Position(pos));
				return true;
			}
		}
	}

	return false;
}

//void BaseAgent::_deprecated_setSquadID(int id)
//{
//	ERROR_MESSAGE(false, "Called _deprecated_setSquadID(). This function will be removed in the future");
//}
//
//int BaseAgent::_deprecated_getSquadID()
//{
//	ERROR_MESSAGE(false, "Called _deprecated_getSquadID(). This function will be removed in the future");
//	return -1;
//}

void BaseAgent::setActionFrame()
{
	lastActionFrame = Broodwar->getFrameCount();
}

int BaseAgent::getLastActionFrame() const
{
	return lastActionFrame;
}

bool BaseAgent::canAttack(const Unit* target) const
{
	return canAttack(target->getType());
}

bool BaseAgent::canAttack(const UnitType& type) const
{
	if (!type.isFlyer())
	{
		if (unit->getType().groundWeapon().targetsGround()) return true;
		if (unit->getType().airWeapon().targetsGround()) return true;
	}
	else
	{
		if (unit->getType().groundWeapon().targetsAir()) return true;
		if (unit->getType().airWeapon().targetsAir()) return true;
	}
	return false;
}

int BaseAgent::noUnitsInWeaponRange() const
{
	int eCnt = 0;
	for(set<Unit*>::const_iterator i=unit->getUnitsInWeaponRange(unit->getType().groundWeapon()).begin();i!=unit->getUnitsInWeaponRange(unit->getType().groundWeapon()).end();i++)
	{
		if ((*i)->exists() && (*i)->getPlayer()->getID() != Broodwar->self()->getID())
		{
			eCnt++;
		}
	}
	return eCnt;
}

void BaseAgent::setGoal(const TilePosition& goal)
{
	if (unit->getType().isFlyer() || unit->getType().isFlyingBuilding())
	{
		//Fliers, can always move to goals.
		this->goal = goal;
	}
	else
	{
		//Ground units, check if we can reach goal.
		if (bats::ExplorationManager::canReach(this, goal))
		{
			this->goal = goal;
		}
	}
}

void BaseAgent::clearGoal()
{
	goal = TilePositions::Invalid;
}

const TilePosition& BaseAgent::getGoal() const
{
	return goal;
}

bool BaseAgent::isWeaponCooldown() const {
	return unit->getGroundWeaponCooldown() > 0 || unit->getAirWeaponCooldown() > 0;
}

void BaseAgent::printGraphicDebugSelectedUnits() const {
	if (bats::config::debug::GRAPHICS_VERBOSITY == bats::config::debug::GraphicsVerbosity_Off ||
		bats::config::debug::modules::AGENT_SELECTED == false)
	{
		return;
	}

	// Only print debug info if the unit is selected
	if (unit->isSelected()) {
		const string& info = getDebugString();
		const Position& unitPos = unit->getPosition();
		Broodwar->drawTextMap(unitPos.x(), unitPos.y(), "%s", info.c_str());
	}
}

string BaseAgent::getDebugString() const {
	stringstream ss;
	ss << bats::TextColors::LIGHT_GREY << left;

	// Low
	// Id
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Low) {
		ss << setw(bats::config::debug::GRAPHICS_COLUMN_WIDTH) << "Id: " << unitID << "\n";
	}

	// High
	// Position
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_High) {
		ss << setw(bats::config::debug::GRAPHICS_COLUMN_WIDTH) << "Pos: " << unit->getTilePosition() << "\n";
	}
	
	return ss.str();
}