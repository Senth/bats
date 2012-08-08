#include "ComsatAgent.h"
#include "WorkerAgent.h"
#include "AgentManager.h"

using namespace BWAPI;
using namespace std;

ComsatAgent::ComsatAgent(Unit* mUnit) : StructureAgent(mUnit)
{
	agentType = "ComsatAgent";
}

void ComsatAgent::computeActions()
{
	handleUnderAttack();

	for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
	{
		//Enemy seen
		if ((*i)->exists())
		{
			if ((*i)->isCloaked() || (*i)->isBurrowed())
			{
				if (unit->getEnergy() >= 50) 
				{
					unit->useTech(TechTypes::Scanner_Sweep, (*i)->getPosition());
				}
			}
		}
	}
}
