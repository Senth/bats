#include "CorsairAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

CorsairAgent::CorsairAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "CorsairAgent";
	//Broodwar->printf("CorsairAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
	lastUseFrame = Broodwar->getFrameCount();
}

void CorsairAgent::computeActions()
{
	if (chargeShields())
	{
		return;
	}

	if (Broodwar->getFrameCount() - lastUseFrame >= 80)
	{
		TechType web = TechTypes::Disruption_Web;
		if (Broodwar->self()->hasResearched(web))
		{
			if (unit->getEnergy() >= 125)
			{
				Unit* target = getClosestEnemyAirDefense(320);
				if (target != NULL)
				{
					if (target->getEnsnareTimer() == 0)
				{
						unit->useTech(web, target);
						lastUseFrame = Broodwar->getFrameCount();
						//Broodwar->printf("[%d] Using Disruption Web on %s", unitID, target->getType().getName().c_str());
						return;
					}
				}
			}
		}
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, false);
}
