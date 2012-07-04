#include "MedicAgent.h"
#include "PFManager.h"
#include "AgentManager.h"

using namespace BWAPI;
using namespace std;

MedicAgent::MedicAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "MedicAgent";
}

void MedicAgent::computeActions()
{
	if (checkUnitsToHeal())
	{
		return;
	}

	computeAttackingActions(true);
}

bool MedicAgent::checkUnitsToHeal()
{
	try {
		double bestDist = -1;
		Unit* toHeal = NULL;

		vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
		for (int i = 0; i < (int)agents.size(); i++)
		{
			BaseAgent* agent = agents.at(i);
			if (agent->isAlive() && agent->isDamaged())
			{
				if (isMedicTarget(agent->getUnit()) && agent->getUnitID() != unit->getID())
				{
					Unit* cUnit = agent->getUnit();
					if (cUnit->exists() && cUnit->getHitPoints() > 0)
				{
						double dist = unit->getDistance(cUnit);
						if (bestDist < 0 || dist < bestDist)
				{
							bestDist = dist;
							toHeal = cUnit;
						}
					}
				}
			}
		}

		if (bestDist >= 0 && toHeal !=NULL)
		{
			//Broodwar->printf("[%d] Medic healing", unitID);
			unit->useTech(TechTypes::Healing, toHeal);
		}
	}
	catch(exception)
	{
		Broodwar->printf("[%d] checkUnitToHeal() error", unit->getID());
	}

	return false;
}

bool MedicAgent::checkUnitsToFollow()
{
	double bestDist = -1;
	Unit* toFollow = NULL;

	/*vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		if (agents.at(i)->isAlive())
		{
			Unit* cUnit = agents.at(i)->getUnit();
			if (isMedicTarget(cUnit))
			{
				double dist = unit->getDistance(cUnit);
				if (bestDist < 0 || dist < bestDist)
				{
					bestDist = dist;
					toFollow = cUnit;
				}
			}
		}
	}*/
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		Unit* cAgent = agents.at(i)->getUnit();
		if (agents.at(i)->isAlive() && isMedicTarget(cAgent))
		{
			double dist = unit->getDistance(cAgent);
			if (bestDist < 0 || dist < bestDist)
			{
				bestDist = dist;
				toFollow = cAgent;
			}
		}
	}

	if (bestDist >= 128)
	{
		unit->follow(toFollow);
		return true;
	}
	else if (bestDist >= 0)
	{
		unit->stop();
		return true;
	}

	return false;
}

bool MedicAgent::isMedicTarget(Unit* mUnit)
{
	if (!mUnit->getType().isOrganic())
	{
		//Can only heal organic units
		return false;
	}

	if (mUnit->getType().isWorker())
	{
		//We can heal workers, but no point
		//in following them
		return false;
	}

	if (!mUnit->getType().canAttack())
	{
		//Dont follow units that cant attack
		return false;
	}

	if (isOfType(mUnit, UnitTypes::Terran_Medic))
	{
		//Dont follow other medics
		return false;
	}

	if (mUnit->isLoaded())
	{
		//Dont "follow" bunkered units
		return false;
	}

	//Check if the unit is exploring, then dont follow it.
	BaseAgent* agent = AgentManager::getInstance()->getAgent(mUnit->getID());
	if (agent->isExploring())
	{
		return false;
	}
	return true;
}
