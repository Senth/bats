#include "SelfClassifier.h"
#include "BuildPlanner.h"
#include "ResourceGroup.h"
#include "Config.h"
#include "SquadManager.h"
#include "AttackSquad.h"
#include "SquadDefs.h"
#include "GameTime.h"
#include "BTHAIModule/Source/AgentManager.h"
#include "BTHAIModule/Source/CommandCenterAgent.h"
#include <BWAPI/Game.h>

using namespace bats;
using namespace BWAPI;
using namespace std;

SelfClassifier* SelfClassifier::msInstance = NULL;

SelfClassifier::SelfClassifier() {
	mAgentManager = NULL;
	mBuildPlanner = NULL;
	mSquadManager = NULL;
	mGameTime = NULL;

	mAgentManager = AgentManager::getInstance();
	mBuildPlanner = BuildPlanner::getInstance();
	mSquadManager = SquadManager::getInstance();
	mGameTime = GameTime::getInstance();
}

SelfClassifier::~SelfClassifier() {
	msInstance = NULL;
}

SelfClassifier* SelfClassifier::getInstance() {
	if (NULL == msInstance) {
		msInstance = new SelfClassifier();
	}
	return msInstance;
}

bool SelfClassifier::isExpanding() const {
	// Return the correct base for each race
	UnitType baseType = Broodwar->self()->getRace().getCenter();

	// Check with the build planner if we're building any expansion
	if (mBuildPlanner->countInProduction(baseType) > 0) {
		return true;
	} else if (mBuildPlanner->nextIsOfType(baseType)) {
		return true;
	} else {
		return false;
	}
}

#pragma warning(push)
#pragma warning(disable: 4100)
bool SelfClassifier::isUpgradeSoonDone(const std::vector<BWAPI::UnitType>& affectedUnitTypes) const {
	/// @todo isUpgradeSoonDone, waiting for upgrades in BuildPlanner.
	return true;
}
#pragma warning(pop)

bool SelfClassifier::areExpansionsSaturated() const {
	int cWorkers = mAgentManager->getNoWorkers();
	int cMineralPatches = 0;

	// Get number of resources
	const vector<BaseAgent*>& agents = mAgentManager->getAgents(Broodwar->self()->getRace().getCenter());
	for (size_t i = 0; i < agents.size(); ++i) {
		/// @todo use SturctureMainAgent instead
		CommandCenterAgent* mainStructure = dynamic_cast<CommandCenterAgent*>(agents[i]);

		if (NULL != mainStructure) {
			ResourceGroupCstPtr resourceGroup = mainStructure->getResourceGroup();
			cMineralPatches += resourceGroup->getActiveMineralPatchCount();
		}
	}

	int saturationWorkerValue = static_cast<int>(cMineralPatches * config::classification::expansion::WORKERS_PER_MINERAL_SATURATION);

	return cWorkers >= saturationWorkerValue;
}

bool SelfClassifier::isAnExpansionLowOnMinerals() const {
	// Get all expansions
	const vector<BaseAgent*>& agents = mAgentManager->getAgents(Broodwar->self()->getRace().getCenter());
	for (size_t i = 0; i < agents.size(); ++i) {
		/// @todo use SturctureMainAgent instead
		CommandCenterAgent* mainStructure = dynamic_cast<CommandCenterAgent*>(agents[i]);

		if (NULL != mainStructure) {
			ResourceGroupCstPtr resourceGroup = mainStructure->getResourceGroup();
			double fractionLeft = resourceGroup->getResourcesLeftInFraction();

			if (fractionLeft > 0.0 && fractionLeft <= config::classification::expansion::EXPANSION_MINERALS_LOW) {
				return true;
			}
		}
	}

	return false;
}

int SelfClassifier::getActiveExpansionCount() const {
	int cExpansions = 0;

	// Get all expansions
	const vector<BaseAgent*>& agents = mAgentManager->getAgents(Broodwar->self()->getRace().getCenter());
	for (size_t i = 0; i < agents.size(); ++i) {
		/// @todo use SturctureMainAgent instead
		CommandCenterAgent* mainStructure = dynamic_cast<CommandCenterAgent*>(agents[i]);

		if (NULL != mainStructure) {
			if (!hasExpansionLowOrNoMinerals(mainStructure)) {
				++cExpansions;
			}
		}
	}


	return cExpansions;
}

bool SelfClassifier::hasExpansionLowOrNoMinerals(const CommandCenterAgent* mainStructure) const {
	ResourceGroupCstPtr resourceGroup = mainStructure->getResourceGroup();
	return resourceGroup->getResourcesLeftInFraction() <= config::classification::expansion::EXPANSION_MINERALS_LOW;
}

bool SelfClassifier::isAttacking() const {
	const std::vector<AttackSquadPtr>& attackSquads = mSquadManager->getSquads<AttackSquad>();
	
	for (size_t i = 0; i < attackSquads.size(); ++i) {
		if (!attackSquads[i]->isRetreating()) {
			return true;
		}
	}

	return false;
}

double SelfClassifier::getLastExpansionStartTime() const {
	double lastExpansionFrame = 0;

	// Get all expansions
	const vector<BaseAgent*>& agents = mAgentManager->getAgents(Broodwar->self()->getRace().getCenter());
	for (size_t i = 0; i < agents.size(); ++i) {
		if (agents[i]->getCreationFrame() > lastExpansionFrame) {
			lastExpansionFrame = agents[i]->getCreationFrame();
		}
	}

	return mGameTime->getElapsedTime(lastExpansionFrame);
}