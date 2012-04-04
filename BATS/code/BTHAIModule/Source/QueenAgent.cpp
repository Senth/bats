#include "QueenAgent.h"
#include "PFManager.h"
#include "AgentManager.h"

using namespace BWAPI;
using namespace std;

QueenAgent::QueenAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "QueenAgent";
	//Broodwar->printf("QueenAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void QueenAgent::computeActions()
{
	if (Broodwar->self()->hasResearched(TechTypes::Spawn_Broodlings))
	{
		if (checkSpawnBroodlings())
		{
			return;
		}
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, true);
}

bool QueenAgent::checkSpawnBroodlings()
{
	if (unit->getEnergy() < 150)
	{
		return false;
	}

	int eCnt = 0;
	int eScore = 0;
	int maxRange = 9 * 32;

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		if ((*i)->exists())
		{
			if ((*i)->getType().canAttack())
			{
				double dist = (*i)->getDistance(unit->getPosition());
				if (dist <= maxRange)
				{
					eCnt++;
					eScore += (*i)->getType().destroyScore();
				}
			}
		}
	}
	
	if (eCnt > 5)
	{
		Unit* target = this->getClosestOrganicEnemy(maxRange);
		if (target != NULL)
		{
			unit->useTech(TechTypes::Spawn_Broodlings, target);
			//Broodwar->printf("Used Spawn Broodlings on %s", target->getType().getName().c_str());
			return true;
		}
	}
	
	return false;
}
