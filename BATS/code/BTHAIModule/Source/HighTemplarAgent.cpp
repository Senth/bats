#include "HighTemplarAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "Commander.h"

using namespace BWAPI;
using namespace std;

HighTemplarAgent::HighTemplarAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "HighTemplarAgent";
	//Broodwar->printf("HighTemplarAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
	hasCastTransform = false;
}

void HighTemplarAgent::computeActions()
{
	if (chargeShields())
	{
		return;
	}

	bool defensive = true;

	if (isOfType(unit->getType(), UnitTypes::Protoss_Archon))
	{
		//High Templar has been transformed to an Archon.
		defensive = false;
	}
	else
	{
		//High Templar unit, use spells
		
		TechType psiStorm = TechTypes::Psionic_Storm;
		if (Broodwar->self()->hasResearched(psiStorm))
		{
			if (unit->getEnergy() >= 75)
			{
				Unit* target = findPsiStormTarget();
				if (target != NULL)
				{
					//Broodwar->printf("[%d] Psionic Storm used on %s", unitID, target->getType().getName().c_str());
					unit->useTech(psiStorm, target->getPosition());
					return;
				}
			}
		}

		TechType hallucination = TechTypes::Hallucination;
		if (Broodwar->self()->hasResearched(hallucination))
		{
			if (unit->getEnergy() >= 100)
			{
				int eCnt = enemyUnitsWithinRange(psiStorm.getWeapon().maxRange());
				if (eCnt >= 4)
				{
					BaseAgent* target = findHallucinationTarget();
					if (target != NULL)
				{
						//Broodwar->printf("[%d] Uses Hallucination on %s", unitID, target->getUnitType().getName().c_str());
						unit->useTech(hallucination, target->getUnit());
						return;
					}
				}
			}
		}

		if (unit->getRemainingBuildTime() == 0)
		{
			Squad* sq = Commander::getInstance()->getSquad(squadID);
			if (sq != NULL)
			{
				if (sq->morphsTo().getID() == UnitTypes::Protoss_Archon.getID())
				{
					if (noUnitsInWeaponRange() == 0)
				{
						BaseAgent* target = findArchonTarget();
						if (target != NULL)
				{
							unit->useTech(TechTypes::Archon_Warp, target->getUnit());
							hasCastTransform = true;
							return;
						}
					}
				}
			}
		}

		//Out of Energy, see if we can transform to an Archon
		if (unit->getEnergy() <= 50 && enemyUnitsWithinRange(320) > 0)
		{
			if (!hasCastTransform)
			{
				BaseAgent* target = findArchonTarget();
				if (target != NULL)
				{
					unit->useTech(TechTypes::Archon_Warp, target->getUnit());
					hasCastTransform = true;
					//Broodwar->printf("[%d] %s uses Summon Archon on %s", unitID, unit->getType().getName().c_str(), target->getUnitType().getName().c_str());
					return;
				}
			}
		}
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
}

Unit* HighTemplarAgent::findPsiStormTarget()
{
	int maxRange = 6*32;

	Unit* target = NULL;
	int bestScore = 150;

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{		
		if ((*i)->exists() && (*i)->getType().canAttack())
		{
			double dist = unit->getDistance((*i)->getPosition());
			if (dist <= maxRange)
			{
				int cScore = (*i)->getType().destroyScore();
				if (bestScore < 0 || cScore > bestScore)
				{
					if (friendlyUnitsWithinRange((*i)->getTilePosition(), 64) == 0 && !(*i)->isUnderStorm())
				{
						bestScore = cScore;
						target = (*i);
					}
				}
			}
		}
	}

	return target;
}

BaseAgent* HighTemplarAgent::findHallucinationTarget()
{
	int maxRange = TechTypes::Hallucination.getWeapon().maxRange();

	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);

		bool targetUnit = false;
		if (agent->isOfType(UnitTypes::Protoss_Carrier)) targetUnit = true;
		if (agent->isOfType(UnitTypes::Protoss_Scout)) targetUnit = true;
		if (agent->isOfType(UnitTypes::Protoss_Archon)) targetUnit = true;
		if (agent->isOfType(UnitTypes::Protoss_Reaver)) targetUnit = true;

		if (agent->isAlive() && targetUnit)
		{
			if (!agent->getUnit()->isHallucination())
			{
				return agent;
			}
		}
	}
	
	return NULL;
}

BaseAgent* HighTemplarAgent::findArchonTarget()
{
	Squad* mSquad = Commander::getInstance()->getSquad(squadID);
	if (mSquad != NULL)
	{
		vector<BaseAgent*> agents = mSquad->getMembers();
		for (int i = 0; i < (int)agents.size(); i++)
		{
			BaseAgent* agent = agents.at(i);
			if (agent->isAlive() && agent->getUnitID() != unitID && agent->isOfType(UnitTypes::Protoss_High_Templar) && !agent->getUnit()->isBeingConstructed())
			{
				double dist = agent->getUnit()->getPosition().getDistance(unit->getPosition());
				if (dist <= 64)
				{
					return agent;
				}
			}
		}
	}

	return NULL;
}
