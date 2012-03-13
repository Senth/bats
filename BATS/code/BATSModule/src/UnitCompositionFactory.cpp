#include "UnitCompositionFactory.h"

using namespace bats;

UnitComposition UnitCompositionFactory::INVALID_UNIT_COMPOSITION = UnitComposition();

UnitCompositionFactory* UnitCompositionFactory::mpsInstance = NULL;

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

}

std::vector<UnitComposition> UnitCompositionFactory::getUnitCompositionsByType(
	std::vector<UnitAgent*> availableUnits,
	UnitCompositions type) const
{
	std::vector<UnitComposition> availableCompositions;

	///@todo 

	return availableCompositions;
}