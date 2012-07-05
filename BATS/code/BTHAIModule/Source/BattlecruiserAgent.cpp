#include "BattlecruiserAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

BattlecruiserAgent::BattlecruiserAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "BattlecruiserAgent";
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

	findAndTryAttack();
	computeMoveAction();
}
