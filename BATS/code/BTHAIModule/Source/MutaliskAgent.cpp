#include "MutaliskAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "Squad.h"
#include "Commander.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

MutaliskAgent::MutaliskAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "MutaliskAgent";
	//Broodwar->printf("MutaliskAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePositions::Invalid;
}

void MutaliskAgent::computeActions()
{
	if (AgentManager::getInstance()->countNoUnits(UnitTypes::Zerg_Greater_Spire) > 0)
	{
		Squad* sq = Commander::getInstance()->getSquad(squadID);
		if (sq != NULL)
		{
			if (sq->morphsTo().getID() == UnitTypes::Zerg_Devourer.getID())
			{
				if (noUnitsInWeaponRange() == 0)
				{
					if (Broodwar->canMake(unit, UnitTypes::Zerg_Devourer))
				{
						unit->morph(UnitTypes::Zerg_Devourer);
						return;
					}
				}
			}

			if (sq->morphsTo().getID() == UnitTypes::Zerg_Guardian.getID())
			{
				if (noUnitsInWeaponRange() == 0)
				{
					if (Broodwar->canMake(unit, UnitTypes::Zerg_Guardian))
				{
						unit->morph(UnitTypes::Zerg_Guardian);
						return;
					}
				}
			}
		}
	}

	Squad* sq = Commander::getInstance()->getSquad(squadID);
	if (sq != NULL)
	{
		if (sq->isKite())
		{
			computeKitingActions();
			return;
		}
	}

	Unit* target = TargetingAgent::findTarget(this);
	if (target != NULL)
	{
		unit->attack(target);
		return;
	}

	bool defensive = useDefensiveMode();
	PFManager::getInstance()->computeAttackingUnitActions(this, goal, defensive);
}
