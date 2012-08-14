#include "TerranProductionBuilding.h"
#include "BTHAIModule/Source/UpgradesPlanner.h"
#include "BuildPlanner.h"
#include "Utilities/Logger.h"

using namespace bats;
using namespace BWAPI;

TerranProductionBuilding::TerranProductionBuilding(BWAPI::Unit* unit) : StructureAgent(unit) {
	agentType = "TerranProductionBuilding";
	DEBUG_MESSAGE(utilities::LogLevel_Fine, "TerranProductionBuilding Created");
}

TerranProductionBuilding::~TerranProductionBuilding(){
}

void TerranProductionBuilding::computeActions(){
	if (isAlive()){
		if (!unit->isIdle()){
			return;
		}
		if (UpgradesPlanner::getInstance()->checkUpgrade(this)){
			return;
		}
		if (!unit->isBeingConstructed() && unit->isIdle() && getUnit()->getType().canProduce()){
			if(!UnitCreator::sLockForQueue){			
				UnitType toBuild = UnitCreator::getInstance()->getNextProducableUnit(unit);
				if(toBuild != UnitTypes::None){
					//Build it!
					DEBUG_MESSAGE(utilities::LogLevel_Finest, "TerranPrudoctionBuilding | Building " <<
						toBuild.getName());
					unit->train(toBuild);
				}
			}
		}
		if (isOfType(UnitTypes::Terran_Starport)){
			if (unit->getAddon() == NULL){
				unit->buildAddon(UnitTypes::Terran_Control_Tower);
			}
		}
		if (isOfType(UnitTypes::Terran_Factory)){
			if (unit->getAddon() == NULL){
				unit->buildAddon(UnitTypes::Terran_Machine_Shop);
			}
		}
	}
}