#include "CommandCenterAgent.h"
#include "AgentManager.h"
#include "WorkerAgent.h"
#include "PFManager.h"
#include "ResourceManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "BatsModule/include/ResourceCounter.h"

using namespace BWAPI;
using namespace std;

bats::ResourceCounter* CommandCenterAgent::msResourceCounter = NULL;

CommandCenterAgent::CommandCenterAgent(Unit* mUnit) : StructureAgent(mUnit)
{
	mHasSentWorkers = false;
	if (AgentManager::getInstance()->countNoUnits(UnitTypes::Terran_Command_Center) == 0)
	{
		//We don't do this for the first Command Center.
		mHasSentWorkers = true;
	}

	agentType = "CommandCenterAgent";

	if (msResourceCounter == NULL) {
		msResourceCounter = bats::ResourceCounter::getInstance();
	}

	// Find closest resource group
	mResourceGroup = msResourceCounter->getClosestResourceGroup(getUnit()->getTilePosition());
}

bats::ResourceGroupCstPtr CommandCenterAgent::getResourceGroup() const {
	return mResourceGroup;
}

void CommandCenterAgent::computeActions()
{
	handleUnderAttack();

	if (!mHasSentWorkers)
	{
		if (isCompleted())
		{
			sendWorkers();
			mHasSentWorkers = true;

			/// @todo check if refinery should be built, depending on our current gas and if this expansion
			/// can hold a refinery.
			bats::BuildPlanner::getInstance()->addRefinery();

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
