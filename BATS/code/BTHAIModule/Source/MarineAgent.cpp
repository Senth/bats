#include "MarineAgent.h"
#include "PFManager.h"
#include "Commander.h"
#include "TargetingAgent.h"
#include "AgentManager.h"
#include "BATSModule/include/SquadManager.h"
#include "BATSModule/include/Squad.h"

using namespace BWAPI;
using namespace std;

MarineAgent::MarineAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "MarineAgent";
	//Broodwar->printf("MarineAgent created (%s)", unit->getType().getName().c_str());
}

void MarineAgent::computeActions()
{
	if (!unit->isLoaded())
	{
		/// @todo check for nearby bunkers to enter
		//Squad* sq = Commander::getInstance()->getSquad(squadID);
		//if (sq != NULL)
		//{
		//	if (sq->isBunkerDefend())
		//	{
		//		vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
		//		for (int i = 0; i < (int)agents.size(); i++)
		//		{
		//			if (agents.at(i)->isAlive() && agents.at(i)->isOfType(UnitTypes::Terran_Bunker))
		//			{
		//				if (agents.at(i)->getUnit()->getLoadedUnits().size() < 4)
		//				{
		//					unit->rightClick(agents.at(i)->getUnit());
		//					return;
		//				}
		//			}
		//		}
		//	}
		//}
	}

	if (Broodwar->self()->hasResearched(TechTypes::Stim_Packs))
	{
		if (!unit->isStimmed() && unit->getHitPoints() >= 20)
		{
			if (unit->isAttacking())
			{
				unit->useTech(TechTypes::Stim_Packs);
				//Broodwar->printf("[%d] Using stim packs", unitID);
				return;
			}
		}
	}

	bool defensive = false;
	if (unit->getGroundWeaponCooldown() > 0)
	{
		defensive = true;
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
}
