#include "CommandCenterAgent.h"
#include "AgentManager.h"
#include "WorkerAgent.h"
#include "PFManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "ResourceManager.h"

using namespace BWAPI;
using namespace std;

CommandCenterAgent::CommandCenterAgent(Unit* mUnit) : StructureAgent(mUnit)
{
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
	handleUnderAttack();

	if (!hasSentWorkers)
	{
		if (isBeingBuilt())
		{
			sendWorkers();
			hasSentWorkers = true;

			//bats::BuildPlanner::getInstance()->addRefinery();

			// Defense manager should handle the adding of bunkers etc.
			//if (AgentManager::getInstance()->countNoUnits(UnitTypes::Terran_Barracks) > 0)
			//{
			//	bats::BuildPlanner::getInstance()->addBuildingFirst(UnitTypes::Terran_Bunker);
			//}
			//if (AgentManager::getInstance()->countNoUnits(UnitTypes::Terran_Engineering_Bay) > 0)
			//{
			//	bats::BuildPlanner::getInstance()->addBuildingFirst(UnitTypes::Terran_Missile_Turret);
			//}
		}
	}

	/// @todo check if refinery should be built, depending on our current gas and if this expansion
	/// can hold a refinery.

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

	/// @todo Unit Creator shall handle this.
	if (ResourceManager::getInstance()->needWorker())
	{
		UnitType worker = Broodwar->self()->getRace().getWorker();
		if (canBuild(worker))
		{
			unit->train(worker);
		}
	}
}
