#include "BattlecruiserAgent.h"
#include "PFManager.h"
#include "AgentManager.h"
#include "TargetingAgent.h"

BattlecruiserAgent::BattlecruiserAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "BattlecruiserAgent";
	//Broodwar->printf("BattlecruiserAgent created (%s)", unit->getType().getName().c_str());
	
	goal = TilePosition(-1, -1);
	lastUseFrame = Broodwar->getFrameCount();
}

void BattlecruiserAgent::computeActions()
{
	if (Broodwar->getFrameCount() - lastUseFrame >= 80)
	{
		TechType gun = TechTypes::Yamato_Gun;
		if (Broodwar->self()->hasResearched(gun))
		{
			if (unit->getEnergy() >= gun.energyUsed())
			{
				int range = gun.getWeapon().maxRange();
				Unit* enemy = getClosestEnemyTurret(range);
				if (enemy != NULL)
				{
					//Broodwar->printf("[%d] Yamato Gun used", unitID);
					unit->useTech(gun, enemy);
					lastUseFrame = Broodwar->getFrameCount();
					return;
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
