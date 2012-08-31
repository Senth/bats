#include "UnitCreator.h"
#include "BuildPlanner.h"
#include "Config.h"
#include "BTHAIModule/Source/BaseAgent.h"
#include "BTHAIModule/Source/ResourceManager.h"
#include "BTHAIModule/Source/AgentManager.h"
#include "Utilities/Logger.h"
#include <vector>
#include <algorithm>
using namespace bats;
using namespace BWAPI;
using namespace std;

int UnitCreator::sMustHave = 0;

UnitCreator::UnitCreator(void){
	mCoreUnitsList = BuildPlanner::getInstance()->getUnitList();
	//TODO: do priotization
	initProductionQueue();	
}

UnitCreator::~UnitCreator(void){
}

void UnitCreator::initProductionQueue(){
	mProductionQueue.clear();
	ProductionQueueItem item;
	sMustHave = 0;
	for(int i=0; i< (int)mCoreUnitsList.size();i++){		
		if(mCoreUnitsList.at(i).mustHave){
			sMustHave++;
			item.mustHave = true;
			item.quantity = mCoreUnitsList.at(i).quantity;
			if(AgentManager::getInstance()->countNoUnits(mCoreUnitsList.at(i).unit) >= mCoreUnitsList.at(i).quantity)
				item.remainingLeft = 0;
			else
				item.remainingLeft = mCoreUnitsList.at(i).quantity;
			item.unit = mCoreUnitsList.at(i).unit;
			mProductionQueue.push_back(item);
		}
	}	
	//verify the current population and initialize the remaining left field accordingly (in case of transitions)
	vector<int> percentage;
	vector <int>::iterator avg_percentage;

	for(int i=0; i < (int)mCoreUnitsList.size();i++){
		if(!mCoreUnitsList.at(i).mustHave)
			percentage.push_back(AgentManager::getInstance()->countNoUnits(mCoreUnitsList.at(i).unit)/mCoreUnitsList.at(i).quantity);		
	}
	avg_percentage = max_element(percentage.begin(),percentage.end());
	for(int i=0; i < (int)mCoreUnitsList.size();i++){
		if(!mCoreUnitsList.at(i).mustHave){
			int p = AgentManager::getInstance()->countNoUnits(mCoreUnitsList.at(i).unit);
			item.mustHave = false;
			item.quantity = mCoreUnitsList.at(i).quantity;
			item.remainingLeft = (((*avg_percentage) * mCoreUnitsList.at(i).quantity) - p) + mCoreUnitsList.at(i).quantity;			
			item.unit = mCoreUnitsList.at(i).unit;
			mProductionQueue.push_back(item);
		}
	}	
}

UnitCreator* UnitCreator::instance = NULL;
bool UnitCreator::sLockForQueue = false;

UnitCreator* UnitCreator::getInstance(){
	if(instance == NULL)
		instance = new UnitCreator();
	return instance;
}

void UnitCreator::updateProductionQueue(){
	// check if we have building to construct other units in the queue, else update the count
	for(int i=0; i < (int)mProductionQueue.size();i++){
		if(mProductionQueue.at(i).mustHave)
			continue;
		if(mProductionQueue.at(i).remainingLeft != 0){
			std::vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
			for (int j = 0; j < (int)agents.size(); j++){
				if(agents.at(j)->isBuilding())
				if (agents.at(j)->isAlive() && agents.at(j)->getUnit()->isIdle() && agents.at(j)->canBuild(mProductionQueue.at(i).unit))
					return;
			}
		}
	}

	// All percentage units are done once, so reinitialize them again
	for(int i=0; i < (int)mProductionQueue.size();i++){
		if(mProductionQueue.at(i).mustHave)
			continue;
		if(mProductionQueue.at(i).remainingLeft == 0)
			mProductionQueue.at(i).remainingLeft = mProductionQueue.at(i).quantity;
	}
}

void UnitCreator::updatePopulation(BWAPI::UnitType unitType){
	bool checkTerranTankSpecialCase = false;
	if(unitType == UnitTypes::Terran_Siege_Tank_Siege_Mode || unitType == UnitTypes::Terran_Siege_Tank_Tank_Mode)
		checkTerranTankSpecialCase = true;
	for(int i=0; i < (int)mProductionQueue.size();i++){
		bool unitMatches = false;
		if(checkTerranTankSpecialCase){
			if(mProductionQueue.at(i).unit == UnitTypes::Terran_Siege_Tank_Siege_Mode 
				|| mProductionQueue.at(i).unit == UnitTypes::Terran_Siege_Tank_Tank_Mode){
				unitMatches = true;
				checkTerranTankSpecialCase = false;
			}
		}

		if(mProductionQueue.at(i).unit == unitType || unitMatches){
			if(mProductionQueue.at(i).mustHave){
				//check the overall unit population
				int total = AgentManager::getInstance()->countNoUnits(unitType);
				if(total < mProductionQueue.at(i).quantity)
				mProductionQueue.at(i).remainingLeft = mProductionQueue.at(i).quantity - total;
				return;
			}
			mProductionQueue.at(i).remainingLeft++;
			return;
		}
	}
}

bool UnitCreator::compareByPriority(ProductionQueueItem &a, ProductionQueueItem &b){
	/// @todo fix priority
	float p1,p2;
	p1 = (float) a.remainingLeft / a.quantity;
	p2 = (float) b.remainingLeft / b.quantity;
	//if(canProceedToNextUnit(a.unit))
		//return false;
	//else
		return p1 > p2;
}

BWAPI::UnitType UnitCreator::getNextProducableUnit(BWAPI::Unit* builder){
	if(sLockForQueue)
		return UnitTypes::None;
	sLockForQueue = true;
	BWAPI::UnitType type;
	
	//TODO check the percentage goal and make priority, if 
	sort(mProductionQueue.begin()+sMustHave, mProductionQueue.end(), UnitCreator::compareByPriority);

	for(int i=0; i < (int)mProductionQueue.size();i++){
		if(mProductionQueue.at(i).remainingLeft == 0){
			if(mProductionQueue.at(i).mustHave)
				continue;
			else{
				// one round of production for this unit is done, so move to next
				updateProductionQueue();
				continue;
			}
		}
			
		type = mProductionQueue.at(i).unit;
		// TODO check for all the iteration when we reach maximum factor on the resources (minerals, gas and supply)
		if(!canProceedToNextUnit(type))
		if (!ResourceManager::getInstance()->hasResources(type)){
			sLockForQueue = false;
			return UnitTypes::None;
		}
		int supplyTotal = Broodwar->self()->supplyTotal() / 2;
		int supplyUsed = Broodwar->self()->supplyUsed() / 2;
		if(supplyUsed >= supplyTotal){
			sLockForQueue = false;
			return UnitTypes::None;
		}		

		if (BWAPI::Broodwar->canMake(builder, type)){
			mProductionQueue.at(i).remainingLeft--;
			std::string temp = type.getName();
			sLockForQueue = false;
			return type;
		}
		
		if(!canProceedToNextUnit(type)){
			sLockForQueue = false;
			return UnitTypes::None;
		}

		//check if we need to put hold on the queue, in scenario where no units can be build
		int noOfUnitsCannotBeBuild = 0;
		for(int j=0; j < (int)mProductionQueue.size(); j++){
			if(mProductionQueue.at(j).mustHave)
				if(mProductionQueue.at(j).remainingLeft == 0){
					noOfUnitsCannotBeBuild ++;
					continue;
				}
			if(canProceedToNextUnit(mProductionQueue.at(j).unit))
				noOfUnitsCannotBeBuild ++;
		}
		if(noOfUnitsCannotBeBuild == (int)mProductionQueue.size()){
			sLockForQueue = false;
			//if number of units cannot be build is same as the size of the queue, then we know that no units can be build
			return UnitTypes::None;
		}

		// skipping to the low priority unit since no suitable building is found to produce this unit, also increase the remaining left count
		//if(!mProductionQueue.at(i).mustHave)
			//mProductionQueue.at(i).remainingLeft += mProductionQueue.at(i).quantity;
	}
	sLockForQueue = false;
	return UnitTypes::None;
}

/*
* check if this unit "unitType" can be built by any other building
* @return false if this unit is buildable by any other building (unit creator wont proceed to next unit in queue)
* @return true if there is a building that can build this unit
*/
bool UnitCreator::canProceedToNextUnit(BWAPI::UnitType unitType){	
	std::vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++){
		if (agents.at(i)->isAlive() && agents.at(i)->getUnit()->isIdle() && agents.at(i)->canBuild(unitType)){
			//agents.at(i)->getUnit()->isIdle();
			return false;
		}
	}
	return true;
}
void UnitCreator::printGraphicDebugInfo(){
	// Low
	// Print queue
	if(config::debug::GRAPHICS_VERBOSITY >= config::debug::GraphicsVerbosity_Low) {
		int line = 1, y = 100;
		std::stringstream info;
	
		Broodwar->drawTextScreen(5,y,"UnitCreator:");
	
		for (int i = 0; i < (int)mProductionQueue.size(); i++){
			info << mProductionQueue.at(i).unit.getName() + " : ";
			info << mProductionQueue.at(i).quantity;
			info << " : ";
			info << mProductionQueue.at(i).remainingLeft;
			Broodwar->drawTextScreen(5,y+16*line, info.str().c_str());
			info.str("");
			info.clear();
			line++;
		}
	}
}
void UnitCreator::switchPhase(){
	sLockForQueue = true;
	mCoreUnitsList = BuildPlanner::getInstance()->getUnitList();
	initProductionQueue();
	sLockForQueue = false;
}