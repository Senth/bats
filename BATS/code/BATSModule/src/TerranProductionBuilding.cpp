#include "TerranProductionBuilding.h"
#include "BuildPlanner.h"
#include "Utilities/Logger.h"

using namespace bats;
using namespace BWAPI;

TerranProductionBuilding::TerranProductionBuilding(BWAPI::Unit* unit) : StructureAgent(unit) {
	if(unit->getType() == UnitTypes::Terran_Barracks)
		agentType = "TerranProductionBuilding_Barracks";
	else if(unit->getType() == UnitTypes::Terran_Factory)
		agentType = "TerranProductionBuilding_Factory";
	else if(unit->getType() == UnitTypes::Terran_Starport)
		agentType = "TerranProductionBuilding_Starport";
	DEBUG_MESSAGE(utilities::LogLevel_Fine, "TerranProductionBuilding Created");
}

TerranProductionBuilding::~TerranProductionBuilding(){
}

void TerranProductionBuilding::computeActions(){
	if (isAlive()){
		if (!unit->isIdle()){
			return;
		}

		if (isOfType(UnitTypes::Terran_Starport)){
			if (unit->getAddon() == NULL){
				unit->buildAddon(UnitTypes::Terran_Control_Tower);
				return;
			}
		}
		if (isOfType(UnitTypes::Terran_Factory)){
			if (unit->getAddon() == NULL){
				unit->buildAddon(UnitTypes::Terran_Machine_Shop);
				return;
			}
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

	}
}