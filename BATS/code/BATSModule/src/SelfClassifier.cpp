#include "SelfClassifier.h"
#include "BuildPlanner.h"
#include "ResourceGroup.h"
#include "Config.h"
#include "SquadManager.h"
#include "AttackSquad.h"
#include "SquadDefs.h"
#include "ScoutSquad.h"
#include "GameTime.h"
#include "BTHAIModule/Source/UnitAgent.h"
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
	} else if (mBuildPlanner->containsType(baseType)) {
		return true;
	} else {
		return false;
	}
}

bool SelfClassifier::isUpgradeSoonDone(const std::vector<UnitAgent*>& affectedUnits) const {
	// Get affected unit types
	set<UnitType> affectedUnitTypes;
	for (size_t i = 0; i < affectedUnits.size(); ++i) {
		affectedUnitTypes.insert(affectedUnits[i]->getUnitType());
	}

	// Get affected upgrades
	set<UpgradeType> affectedUpgradeTypes;
	set<UnitType>::const_iterator unitTypeIt;
	for (unitTypeIt = affectedUnitTypes.begin(); unitTypeIt != affectedUnitTypes.end(); ++unitTypeIt) {
		affectedUpgradeTypes.insert(unitTypeIt->upgrades().begin(), unitTypeIt->upgrades().end());
	}


	// Is any building upgrading? If so, is it soon done and will it affect any of the units?
	set<Unit*> ourUnits = Broodwar->self()->getUnits();
	set<Unit*>::const_iterator unitIt;
	for (unitIt = ourUnits.begin(); unitIt != ourUnits.end(); ++unitIt) {
		UpgradeType upgradeType = (*unitIt)->getUpgrade();
		if (upgradeType != UpgradeTypes::None) {
			set<UpgradeType>::const_iterator foundIt = affectedUpgradeTypes.find(upgradeType);
			if (foundIt != affectedUpgradeTypes.end()) {

				// Is the upgrade soon done?
				double timeLeft = mGameTime->convertFramesToSeconds((*unitIt)->getRemainingUpgradeTime());
				if (timeLeft <= config::classification::UPGRADE_SOON_DONE) {
					return true;
				}
			}
		}
	}

	return false;
}

bool SelfClassifier::areExpansionsSaturated() const {
	int cWorkers = mAgentManager->getMiningWorkerCount();
	int cMineralPatches = 0;

	// Get number of resources
	const vector<const BaseAgent*>& agents = mAgentManager->getAgents(Broodwar->self()->getRace().getCenter());
	for (size_t i = 0; i < agents.size(); ++i) {
		/// @todo use SturctureMainAgent instead
		const CommandCenterAgent* mainStructure = dynamic_cast<const CommandCenterAgent*>(agents[i]);

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
	const vector<const BaseAgent*>& agents = mAgentManager->getAgents(Broodwar->self()->getRace().getCenter());
	for (size_t i = 0; i < agents.size(); ++i) {
		/// @todo use StructureMainAgent instead
		const CommandCenterAgent* mainStructure = dynamic_cast<const CommandCenterAgent*>(agents[i]);

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
	const vector<const BaseAgent*>& agents = mAgentManager->getAgents(Broodwar->self()->getRace().getCenter());
	for (size_t i = 0; i < agents.size(); ++i) {
		/// @todo use SturctureMainAgent instead
		const CommandCenterAgent* mainStructure = dynamic_cast<const CommandCenterAgent*>(agents[i]);

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
	const std::vector<AttackSquadCstPtr>& attackSquads = mSquadManager->getSquads<AttackSquad>();
	
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
	const vector<const BaseAgent*>& agents = mAgentManager->getAgents(Broodwar->self()->getRace().getCenter());
	for (size_t i = 0; i < agents.size(); ++i) {
		if (agents[i]->getCreationFrame() > lastExpansionFrame) {
			lastExpansionFrame = agents[i]->getCreationFrame();
		}
	}

	return mGameTime->getElapsedTime(lastExpansionFrame);
}

bool SelfClassifier::isScouting() const {
	const std::vector<ScoutSquadCstPtr>& scoutSquads = mSquadManager->getSquads<ScoutSquad>();

	return !scoutSquads.empty();
}

bool SelfClassifier::isHighOnMinerals() const {
	return Broodwar->self()->minerals() >= config::classification::HIGH_ON_MINERALS;
}

bool SelfClassifier::isHighOnGas() const {
	return Broodwar->self()->gas() >= config::classification::HIGH_ON_GAS;
}

bool SelfClassifier::isHighOnResources() const {
	return isHighOnMinerals() && isHighOnGas();
}