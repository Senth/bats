#include "ScienceVesselAgent.h"
#include "PFManager.h"
#include "AgentManager.h"

using namespace BWAPI;
using namespace std;

ScienceVesselAgent::ScienceVesselAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "ScienceVesselAgent";
	lastIrradiateFrame = 0;
}

void ScienceVesselAgent::computeActions()
{
	if (isUnderAttack() && !unit->isDefenseMatrixed() && unit->getEnergy() >= 100)
	{
		unit->useTech(TechTypes::Defensive_Matrix, unit);
		return;
	}

	//Check other important units that might need shielding
	if (unit->getEnergy() >= 100)
	{
		BaseAgent* agent = findImportantUnit();
		if (agent != NULL)
		{
			unit->useTech(TechTypes::Defensive_Matrix, agent->getUnit());
		}
	}

	if (Broodwar->getFrameCount() - lastIrradiateFrame < 200)
	{ //Dont cast it too often
		TechType irradiate = TechTypes::Irradiate;
		if (Broodwar->self()->hasResearched(irradiate))
		{
			if (unit->getEnergy() >= 75)
			{
				int range = irradiate.getWeapon().maxRange();
				int eCnt = enemyUnitsWithinRange(range);
				if (eCnt >= 5)
				{
					//Only use Irradiate if we have a couple of
					//enemy units around
					Unit* enemy = getClosestOrganicEnemy(range);
					if (enemy != NULL)
				{
						//Be sure that the targeted unit isnt already
						//irradiated
						if (enemy->getIrradiateTimer() == 0)
				{
							//Broodwar->printf("[%d] Irradiate used", unitID);
							unit->useTech(irradiate, enemy);
							lastIrradiateFrame = Broodwar->getFrameCount();
							return;
						}
					}
				}

			}
		}
	}

	TechType emp = TechTypes::EMP_Shockwave;
	if (Broodwar->self()->hasResearched(emp))
	{
		if (unit->getEnergy() >= emp.energyUsed())
		{
			int range = emp.getWeapon().maxRange();
			Unit* enemy = getClosestShieldedEnemy(range);
			if (enemy != NULL)
			{
				//Broodwar->printf("[%d] EMP Shockwave used", unitID);
				unit->useTech(emp, enemy->getPosition());
				return;
			}
		}
	}

	computeAttackingActions(true);
}

BaseAgent* ScienceVesselAgent::findImportantUnit()
{
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive() && isImportantUnit(agent) && !agent->getUnit()->isDefenseMatrixed())
		{
			double dist = unit->getDistance(agent->getUnit());
			if (dist <= 320)
			{
				return agent;
			}
		}
	}
	return NULL;
}

bool ScienceVesselAgent::isImportantUnit(BaseAgent* agent)
{
	UnitType type = agent->getUnitType();

	if (agent->isOfType(UnitTypes::Terran_Siege_Tank_Tank_Mode)) return true;
	if (agent->isOfType(UnitTypes::Terran_Siege_Tank_Siege_Mode)) return true;
	if (agent->isOfType(UnitTypes::Terran_Science_Vessel)) return true;
	if (agent->isOfType(UnitTypes::Terran_Battlecruiser)) return true;

	return false;
}
