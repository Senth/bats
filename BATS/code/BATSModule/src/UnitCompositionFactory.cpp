#include "Utilities/Logger.h"
#include "Utilities/MassIniReader.h"
#include "UnitCompositionFactory.h"
#include "Config.h"
#include <BWAPI/Race.h>

using namespace bats;

UnitComposition UnitCompositionFactory::INVALID_UNIT_COMPOSITION = UnitComposition();
UnitCompositionFactory* UnitCompositionFactory::mpsInstance = NULL;

UnitCompositions toCompositionType(const std::string& type) {
	if (type == "drop") {
		return UnitComposition_Drop;
	} else if (type == "scout") {
		return UnitComposition_Scout;
	} else if (type == "harass") {
		return UnitComposition_Harass;
	} else  {
		return UnitComposition_Lim;
	}
}

UnitCompositionFactory::UnitCompositionFactory() {
	reloadConfigs();
}

UnitCompositionFactory::~UnitCompositionFactory() {
	mpsInstance = NULL;
}

UnitCompositionFactory* UnitCompositionFactory::getInstance() {
	if (mpsInstance == NULL) {
		mpsInstance = new UnitCompositionFactory();
	}
	return mpsInstance;
}

void UnitCompositionFactory::reloadConfigs() {
	mUnitCompositions.clear();
	mUnitCompositions.resize(UnitComposition_Lim);

	utilities::MassIniReader unitCompositionReader;

	std::string iniPath = config::UNIT_COMPOSITION_DIR;

	iniPath += "\\" + BWAPI::Broodwar->self()->getRace().getName();

	unitCompositionReader.open(iniPath);

	if (!unitCompositionReader.isOpen()) {
		ERROR_MESSAGE(false, "Could not find folder for unit composition path");
		return;
	}

	std::string typeNameCurrent;
	UnitCompositions typeCurrent;
	std::string nameCurrent;
	std::vector<UnitSet> unitSets;

	while (unitCompositionReader.isGood()) {
		utilities::VariableInfo variableInfo;
		unitCompositionReader.readNext(variableInfo);

		// New Squad, maybe add unit sets
		// The reason we test for new file too is that maybe an equal section in the next file
		// exists
		if (variableInfo.section != nameCurrent || variableInfo.file != typeNameCurrent) {
			if (typeCurrent != UnitComposition_Lim && !unitSets.empty()) {
				mUnitCompositions[typeCurrent].push_back(
					UnitComposition(typeNameCurrent, nameCurrent, unitSets));
			}
		}

		// New file
		if (variableInfo.file != typeNameCurrent) {
			///@todo
		}
	}
}

std::vector<UnitComposition> UnitCompositionFactory::getUnitCompositionsByType(
	std::vector<UnitAgent*> availableUnits,
	UnitCompositions type) const
{
	std::vector<UnitComposition> availableCompositions;

	///@todo 

	return availableCompositions;
}