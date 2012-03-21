#include "Utilities/Logger.h"
#include "Utilities/MassIniReader.h"
#include "UnitCompositionFactory.h"
#include "Config.h"
#include <BWAPI/Race.h>
#include <BWAPI/UnitType.h>

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

	std::string iniPath = config::squad::UNIT_COMPOSITION_DIR;

	iniPath += "\\" + BWAPI::Broodwar->self()->getRace().getName();

	unitCompositionReader.open(iniPath);

	if (!unitCompositionReader.isOpen()) {
		ERROR_MESSAGE(false, "Could not find folder for unit composition path");
		return;
	}

	std::string typeNameCurrent;
	UnitCompositions typeCurrent = UnitComposition_Lim;
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
					UnitComposition(typeNameCurrent, nameCurrent, unitSets)
				);
				unitSets.clear();
			}
		}

		
		// New file
		if (variableInfo.file != typeNameCurrent) {
			typeNameCurrent = variableInfo.file;
			typeCurrent = toCompositionType(typeNameCurrent);

			// No need to read the rest of this file's variables when the current type is invalid
			if (typeCurrent == UnitComposition_Lim) {
				unitCompositionReader.nextFile();
				continue;
			}
		}

		
		// New UnitSet
		BWAPI::UnitType unitType = BWAPI::UnitTypes::getUnitType(variableInfo.name);

		if (unitType != BWAPI::UnitTypes::Unknown) {
			int cUnits = variableInfo;
			if (cUnits > 0) {

				// Check for special morph units, like siege tank.
				bool treatMorphAsSame = false;
				if (unitType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode ||
					unitType == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
				{
					treatMorphAsSame = true;
				}

				unitSets.push_back(UnitSet(unitType, cUnits, treatMorphAsSame));
			}
		}
	}

	// Create the last UnitComposition
	if (typeCurrent != UnitComposition_Lim) {
		mUnitCompositions[typeCurrent].push_back(
			UnitComposition(typeNameCurrent, nameCurrent, unitSets)
		);
	}
}

std::vector<UnitComposition> UnitCompositionFactory::getUnitCompositionsByType(
	std::vector<UnitAgent*> availableUnits,
	UnitCompositions type) const
{
	assert(type >= UnitComposition_First);
	assert(type < UnitComposition_Lim);

	std::vector<UnitComposition> availableCompositions;

	// Create a copy of all unit compositions and check if they become full
	// If they do not become full they are not available unit sets.
	std::vector<UnitComposition> unitCompositions = mUnitCompositions[type];
	for (size_t i = 0; i < unitCompositions.size(); ++i) {
		unitCompositions[i].addUnits(availableUnits);

		if (unitCompositions[i].isFull()) {
			unitCompositions[i].clear();
			availableCompositions.push_back(unitCompositions[i]);
		}
	}

	return availableCompositions;
}