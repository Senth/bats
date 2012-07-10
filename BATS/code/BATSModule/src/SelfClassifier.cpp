#include "SelfClassifier.h"
#include "BTHAIModule/Source/AgentManager.h"
#include "BuildPlanner.h"
#include <cstdlib> // For NULL

using namespace bats;
using namespace BWAPI;
using namespace std;

SelfClassifier* SelfClassifier::mpsInstance = NULL;

SelfClassifier::SelfClassifier() {
	mpAgentManager = NULL;
	mpBuildPlanner = NULL;

	mpAgentManager = AgentManager::getInstance();
	mpBuildPlanner = BuildPlanner::getInstance();
}

SelfClassifier::~SelfClassifier() {
	mpsInstance = NULL;
}

SelfClassifier* SelfClassifier::getInstance() {
	if (NULL == mpsInstance) {
		mpsInstance = new SelfClassifier();
	}
	return mpsInstance;
}

bool SelfClassifier::isExpanding() const {
	// Return the correct base for each race
	UnitType baseType;
	if (Broodwar->self()->getRace() == Races::Terran) {
		baseType = UnitTypes::Terran_Command_Center;
	} else if (Broodwar->self()->getRace() == Races::Protoss) {
		baseType = UnitTypes::Protoss_Nexus;
	} else if (Broodwar->self()->getRace() == Races::Zerg) {
		baseType = UnitTypes::Zerg_Hatchery;
	}

	const vector<BaseAgent*>& bases = mpAgentManager->getAgents(baseType);

	// Check if any of the bases are building
	for (size_t i = 0; i < bases.size(); ++i) {
		if (bases[i]->isBeingBuilt()) {
			return true;
		}
	}


	// Check if next in the build queue is a base
	if (mpBuildPlanner->nextIsOfType(baseType)) {
		return true;
	}


	return false;
}

#pragma warning(push)
#pragma warning(disable: 4100)
bool SelfClassifier::isUpgradeSoonDone(const std::vector<BWAPI::UnitType>& affectedUnitTypes) const {
	/// @todo isUpgradeSoonDone, waiting for upgrades in BuildPlanner.
	return true;
}
#pragma warning(pop)