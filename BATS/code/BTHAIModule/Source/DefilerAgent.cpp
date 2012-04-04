#include "DefilerAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

DefilerAgent::DefilerAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "DefilerAgent";
	//Broodwar->printf("DefilerAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void DefilerAgent::computeActions()
{
	if (checkDarkSwarm())
	{
		return;
	}
	if (Broodwar->self()->hasResearched(TechTypes::Consume))
	{
		if (checkConsume())
		{
			return;
		}
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}
	
	PFManager::getInstance()->computeAttackingUnitActions(this, goal, true);
}

bool DefilerAgent::checkConsume()
{
	if (unit->getEnergy() > 150)
	{
		//Already enough energy. Dont do anything.
		return false;
	}

	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive() && agent->isOfType(UnitTypes::Zerg_Zergling))
		{
			double dist = agent->getUnit()->getTilePosition().getDistance(unit->getTilePosition());
			if (dist <= 2)
			{
				unit->useTech(TechTypes::Consume, agent->getUnit());
				//Broodwar->printf("Used Consume on %s", agent->getUnitType().getName().c_str());
				return true;
			}
		}
	}

	return false;
}

bool DefilerAgent::checkDarkSwarm()
{
	if (unit->getEnergy() < 100)
	{
		return false;
	}

	int maxRange = 9 * 32;

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			UnitType type = (*i)->getType();
			if (type.getID() == UnitTypes::Terran_Missile_Turret.getID() || type.getID() == UnitTypes::Protoss_Photon_Cannon.getID() || type.getID() == UnitTypes::Zerg_Sunken_Colony.getID() || type.getID() == UnitTypes::Zerg_Spore_Colony.getID() || type.getID() == UnitTypes::Terran_Bunker.getID())
			{
				double dist = (*i)->getDistance(unit->getPosition());
				if (dist <= maxRange)
				{
					unit->useTech(TechTypes::Dark_Swarm, (*i)->getPosition());
					//Broodwar->printf("Used Dark Swarm at (%d,%d)", (*i)->getTilePosition().x(), (*i)->getTilePosition().y());
					return true;
				}
			}
		}
	}
	
	return false;
}
