#include "AgentManager.h"
#include "AgentFactory.h"
#include "CoverMap.h"
#include "ResourceManager.h"
#include "WorkerAgent.h"
#include "Profiler.h"
#include "BaseAgent.h"
#include "BatsModule/include/BuildPlanner.h"
#include "BatsModule/include/Commander.h"
#include "BatsModule/include/Helper.h"
using namespace BWAPI;
using namespace std;

int AgentManager::StartFrame = 0;
AgentManager* AgentManager::msInstance = NULL;

AgentManager::AgentManager()
{
	mFrameLastCall = Broodwar->getFrameCount();
}

AgentManager::~AgentManager()
{
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		delete mAgents.at(i);
	}

	msInstance = NULL;
}

AgentManager* AgentManager::getInstance()
{
	if (msInstance == NULL)
	{
		msInstance = new AgentManager();
	}
	return msInstance;
}

const vector<BaseAgent*>& AgentManager::getAgents() {
	return mAgents;
}

const vector<const BaseAgent*>& AgentManager::getAgents() const {
	return *reinterpret_cast<const vector<const BaseAgent*>*>(&mAgents);
}

vector<BaseAgent*> AgentManager::getAgents(const UnitType& unitType) {
	// Use const version
	const AgentManager* pThis = const_cast<const AgentManager*>(this);
	const vector<const BaseAgent*>& agents = pThis->getAgents(unitType);
	return *reinterpret_cast<const vector<BaseAgent*>*>(&agents);
}

vector<const BaseAgent*> AgentManager::getAgents(const UnitType& unitType) const {
	vector<const BaseAgent*> agents;
	for (size_t i = 0; i < mAgents.size(); ++i) {
		if (mAgents[i]->getUnitType() == unitType) {
			agents.push_back(mAgents[i]);
		}
	}

	return agents;
}

int AgentManager::size() const
{
	return mAgents.size();
}

BaseAgent* AgentManager::getAgent(int unitID)
{
	BaseAgent* agent = NULL;

	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->getUnitID() == unitID)
		{
			agent = mAgents.at(i);
			break;
		}
	}

	return agent;
}

void AgentManager::requestOverlord(const TilePosition& pos)
{
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->isOfType(UnitTypes::Zerg_Overlord) && mAgents.at(i)->isAlive())
		{
			if (mAgents.at(i)->getGoal()== TilePositions::Invalid)
			{
				mAgents.at(i)->setGoal(pos);
				return;
			}
		}
	}
}

BaseAgent* AgentManager::getAgent(const UnitType& type)
{
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->isOfType(type) && mAgents.at(i)->isAlive())
		{
			return mAgents.at(i);
		}
	}
	return NULL;
}

BaseAgent* AgentManager::getClosestBase(const TilePosition& pos)
{
	BaseAgent* agent = NULL;
	double bestDist = 100000;

	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->getUnitType().isResourceDepot() && mAgents.at(i)->isAlive())
		{
			double dist = mAgents.at(i)->getUnit()->getDistance(Position(pos));
			if (dist < bestDist)
			{
				bestDist = dist;
				agent = mAgents.at(i);
			}
		}
	}
	return agent;
}

BaseAgent* AgentManager::getClosestAgent(const TilePosition& pos, const UnitType& type)
{
	BaseAgent* agent = NULL;
	double bestDist = 100000;

	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->isOfType(type) && mAgents.at(i)->isAlive())
		{
			double dist = mAgents.at(i)->getUnit()->getDistance(Position(pos));
			if (dist < bestDist)
			{
				bestDist = dist;
				agent = mAgents.at(i);
			}
		}
	}
	return agent;
}

void AgentManager::addAgent(Unit* unit)
{
	if (unit->getType() == UnitTypes::Zerg_Larva)
	{
		//Special case: Don't add Zerg larva as agents.
		return;
	}
	else if (unit->getType() == UnitTypes::Zerg_Egg)
	{
		//Special case: Don't add Zerg eggs as agents.
		return;
	}
	else if (unit->getType() == UnitTypes::Zerg_Cocoon)
	{
		//Special case: Don't add Zerg cocoons as agents.
		return;
	}
	else if (unit->getType() == UnitTypes::Zerg_Lurker_Egg)
	{
		//Special case: Don't add Zerg eggs as agents.
		return;
	}

	bool found = false;
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->matches(unit))
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		BaseAgent* newAgent = AgentFactory::getInstance()->createAgent(unit);
		mAgents.push_back(newAgent);

		onAgentCreated(newAgent);
	}
}

void AgentManager::onAgentCreated(BaseAgent* newAgent) {
	if (newAgent->isBuilding())
	{
		CoverMap::getInstance()->addConstructedBuilding(newAgent->getUnit());
		bats::BuildPlanner::getInstance()->removeFromQueue(newAgent->getUnit()->getType());
		ResourceManager::getInstance()->unlockResources(newAgent->getUnit()->getType());
	}
}

void AgentManager::removeAgent(const Unit* unit)
{
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents[i]->matches(unit))
		{
			onAgentDestroyed(mAgents[i]);	
		}
	}
}

void AgentManager::onAgentDestroyed(BaseAgent* destroyedAgent) {
	if (destroyedAgent->isBuilding())
	{
		CoverMap::getInstance()->buildingDestroyed(destroyedAgent->getUnit());
	}

	destroyedAgent->destroyed();

	return;
}

void AgentManager::morphDrone(Unit* unit)
{
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->matches(unit))
		{
			mAgents.erase(mAgents.begin() + i);
			addAgent(unit);
			return;
		}
	}
	//No match found. Add it anyway.
	if (unit->exists())
	{
		addAgent(unit);
	}
}

void AgentManager::cleanup()
{
	//Step 1. Check if any agent is under attack. If so, dont cleanup since
	//it might cause a Nullpointer.
	//Seems to work now
	/*for (size_t i = 0; i < agents.size(); i++)
	{
		if (agents.at(i)->isUnderAttack())
		{
			return;
		}
	}*/

	//Step 2. Do the cleanup.
	int cnt = 0;
	//size_t oldSize = mAgents.size();
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (!mAgents.at(i)->isAlive())
		{
			delete mAgents.at(i);
			mAgents.erase(mAgents.begin() + i);
			cnt++;
			i--;
		}
	}
	//size_t newSize = mAgents.size();
}

void AgentManager::computeActions()
{
	Profiler::getInstance()->start("AgentManager::update()");

	int st = (int)GetTickCount();
	int et = 0;
	int elapsed = 0;

	for (size_t i = 0; i < mAgents.size(); i++)
	{
		// Don't call too many
		et = (int)GetTickCount();
		elapsed = et - st;
		if (elapsed >= 30)
		{
			return;
		}

		if (mAgents.at(i)->isAlive())
		{
			int lastAF = mAgents.at(i)->getLastActionFrame();
			if (Broodwar->getFrameCount() - lastAF > 20)
			{
				mAgents.at(i)->setActionFrame();
				mAgents.at(i)->computeActions();
			}
		}
	}

	Profiler::getInstance()->end("AgentManager::update()");
}

int AgentManager::getWorkerCount() const
{
	int wCnt = 0;
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		BaseAgent* agent = mAgents.at(i);
		if (agent != NULL && agent->isWorker() && agent->isAlive())
		{
			wCnt++;
		}
	}
	return wCnt;
}

int AgentManager::getMiningWorkerCount() const
{
	int cnt = 0;
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		BaseAgent* agent = mAgents.at(i);
		if (agent->isWorker() && agent->isAlive())
		{
			WorkerAgent* w = (WorkerAgent*)agent;
			if (w->getState() == WorkerAgent::GATHER_MINERALS)
			{
				cnt++;
			}
		}
	}
	return cnt;
}

BaseAgent* AgentManager::findClosestFreeWorker(const TilePosition& pos) {
	BaseAgent* bestWorker = NULL;
	int bestDist = INT_MAX;

	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++) {
		if (agents[i]->isFreeWorker()) {
			int dist = bats::getSquaredDistance(agents[i]->getUnit()->getTilePosition(), pos);
			if (dist < bestDist) {
				bestDist = dist;
				bestWorker = agents[i];
			}
		}
	}

	return bestWorker;
}

bool AgentManager::isAnyAgentRepairingThisAgent(const BaseAgent* repairedAgent) const
{
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		BaseAgent* agent = mAgents.at(i);
		if (agent->isAlive() && agent->isWorker())
		{
			Unit* unit = agent->getUnit();
			if (unit->getTarget() != NULL && unit->getTarget()->getID() == repairedAgent->getUnitID())
			{
				//Already have an assigned builder
				return true;
			}
		}
	}
	return false;
}

int AgentManager::noInProduction(const UnitType& type) const
{
	int cnt = 0;
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->isAlive())
		{
			if (mAgents.at(i)->isOfType(type) && mAgents.at(i)->getUnit()->isBeingConstructed())
			{
				cnt++;
			}
		}
	}
	return cnt;
}

int AgentManager::countNoUnits(const UnitType& type) const
{
	int cnt = 0;
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->isAlive())
		{
			if (mAgents.at(i)->isOfType(type))
			{
				cnt++;
			}
		}
	}
	return cnt;
}

int AgentManager::countNoBases() const
{
	int cnt = 0;
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->isAlive())
		{
			if (mAgents.at(i)->getUnitType().isResourceDepot())
			{
				cnt++;
			}
		}
	}
	return cnt;
}

bool AgentManager::unitsInArea(const TilePosition& pos, int tileWidth, int tileHeight, int unitID) const
{
	for (size_t i = 0; i < mAgents.size(); i++)
	{
		if (mAgents.at(i)->isAlive())
		{
			if (mAgents.at(i)->getUnit()->getID() != unitID)
			{
				TilePosition aPos = mAgents.at(i)->getUnit()->getTilePosition();
				if (aPos.x() >= pos.x() && aPos.x() <= pos.x() + tileWidth && aPos.y() >= pos.y() && aPos.y() <= pos.y() + tileWidth)
				{
					return true;
				}
			}
		}
	}
	return false;
}

TilePosition AgentManager::getClosestDetector(const TilePosition& startPos) const
{
	TilePosition pos = TilePositions::Invalid;
	double bestDist = 10000;

	for (size_t i = 0; i < mAgents.size(); i++)
	{
		BaseAgent* agent = mAgents.at(i);
		if (agent->isAlive())
		{
			if (agent->getUnitType().isDetector() && agent->getUnitType().isBuilding())
			{
				double cDist = startPos.getDistance(agent->getUnit()->getTilePosition());
				if (cDist < bestDist)
				{
					bestDist = cDist;
					pos = agent->getUnit()->getTilePosition();
				}
			}
		}
	}

	return pos;
}

void AgentManager::printGraphicDebugInfo() {
	for (size_t i = 0; i < mAgents.size(); ++i) {
		mAgents[i]->printGraphicDebugInfo();
	}
}

int AgentManager::getUnitProducingStructureCount() const {
	int cStructures = 0;
	bool buildingWorkers = ResourceManager::getInstance()->needWorker();
	for (size_t i = 0; i < mAgents.size(); ++i) {
		UnitType agentType = mAgents[i]->getUnitType();
		if (agentType.canProduce() && agentType.isBuilding() && mAgents[i]->getUnit()->isCompleted()) {
			++cStructures;
		}
	}

	return cStructures;
}