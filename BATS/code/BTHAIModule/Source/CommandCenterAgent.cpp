#include "CommandCenterAgent.h"
#include "AgentManager.h"
#include "WorkerAgent.h"
#include "PFManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "ExplorationManager.h"
#include "ResourceManager.h"

using namespace BWAPI;
using namespace std;

CommandCenterAgent::CommandCenterAgent(Unit* mUnit)
{
	unit = mUnit;
	type = unit->getType();
	unitID = unit->getID();
	//Broodwar->printf("New base created at (%d,%d)", unit->getTilePosition().x(), unit->getTilePosition().y());

	hasSentWorkers = false;
	if (AgentManager::getInstance()->countNoUnits(UnitTypes::Terran_Command_Center) == 0)
	{
		//We dont do this for the first Command Center.
		hasSentWorkers = true;
	}

	agentType = "CommandCenterAgent";
	bats::BuildPlanner::getInstance()->commandCenterBuilt();
}

void CommandCenterAgent::computeActions()
{
	if (!hasSentWorkers)
	{
		if (!unit->isBeingConstructed())
		{
			sendWorkers();
			hasSentWorkers = true;

			bats::BuildPlanner::getInstance()->addRefinery();

			if (AgentManager::getInstance()->countNoUnits(UnitTypes::Terran_Barracks) > 0)
			{
				bats::BuildPlanner::getInstance()->addBuildingFirst(UnitTypes::Terran_Bunker);
			}
			if (AgentManager::getInstance()->countNoUnits(UnitTypes::Terran_Engineering_Bay) > 0)
			{
				bats::BuildPlanner::getInstance()->addBuildingFirst(UnitTypes::Terran_Missile_Turret);
			}
		}
	}

	if (!unit->isIdle())
	{
		//Already doing something
		return;
	}

	//Build comsat addon
	if (unit->getAddon() == NULL)
	{
		if (Broodwar->canMake(unit, UnitTypes::Terran_Comsat_Station))
		{
			unit->buildAddon(UnitTypes::Terran_Comsat_Station);
			return;
		}
	}

	if (ResourceManager::getInstance()->needWorker())
	{
		UnitType worker = Broodwar->self()->getRace().getWorker();
		if (canBuild(worker))
		{
			unit->train(worker);
		}
	}
}
