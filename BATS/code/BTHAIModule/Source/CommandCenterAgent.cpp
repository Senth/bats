#include "CommandCenterAgent.h"
#include "AgentManager.h"
#include "WorkerAgent.h"
#include "PFManager.h"
#include "ResourceManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "BatsModule/include/ResourceCounter.h"
#include "BatsModule/include/UnitHelper.h"
#include "BatsModule/include/SelfClassifier.h"
#include "BatsModule/include/ResourceGroup.h"

using namespace BWAPI;
using namespace std;

bats::ResourceCounter* CommandCenterAgent::msResourceCounter = NULL;

CommandCenterAgent::CommandCenterAgent(Unit* mUnit) : StructureAgent(mUnit)
{
	mGeyser = NULL;

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

	// Check if this expansion can hold a refinery
	const set<BWAPI::Unit*>& geysers = Broodwar->getStaticGeysers();
	set<BWAPI::Unit*>::const_iterator geyserIt = geysers.begin();
	while (NULL == mGeyser && geyserIt != geysers.end()) {
		if ((*geyserIt)->getResourceGroup() == mResourceGroup->getId()) {
			mGeyser = *geyserIt;
		}

		++geyserIt;
	}
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
		}
	}

	// Check if we shall build a Refinery
	if (NULL != mGeyser && mGeyser->getType() == UnitTypes::Resource_Vespene_Geyser) {
		if (!bats::SelfClassifier::getInstance()->isHighOnGas()) {
			bats::BuildPlanner::getInstance()->addRefinery();
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
