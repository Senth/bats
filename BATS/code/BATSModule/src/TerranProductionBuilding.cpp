#include "TerranProductionBuilding.h"
#include "BTHAIModule/Source/UpgradesPlanner.h"
#include "BuildPlanner.h"
#include "Utilities/Logger.h"

using namespace bats;
using namespace BWAPI;

TerranProductionBuilding::TerranProductionBuilding(){
}
TerranProductionBuilding::TerranProductionBuilding(BWAPI::Unit* pUnit){
	unit = pUnit;
	type = unit->getType();
	unitID = unit->getID();
	agentType = "TerranProductionBuilding";
	Broodwar->printf("TerranProductionBuilding Created");
	utilities::printDebugMessage(utilities::LogLevel_Info, "TerranProductionBuilding Created");
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
					Broodwar->printf("%s",toBuild.getName());
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