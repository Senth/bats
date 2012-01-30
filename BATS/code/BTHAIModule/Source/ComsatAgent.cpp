#include "ComsatAgent.h"
#include "WorkerAgent.h"
#include "AgentManager.h"

ComsatAgent::ComsatAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "ComsatAgent";
}

void ComsatAgent::computeActions()
{
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
