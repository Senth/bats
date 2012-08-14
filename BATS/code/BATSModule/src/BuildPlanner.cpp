#include "BuildPlanner.h"
#include "BTHAIModule/Source/WorkerAgent.h"
#include "BTHAIModule/Source/StructureAgent.h"
#include "BTHAIModule/Source/BaseAgent.h"
#include "BTHAIModule/Source/AgentManager.h"
#include "BTHAIModule/Source/CoverMap.h"
#include "BTHAIModule/Source/UnitSetup.h"
#include "BuildOrderFileReader.h"
#include "BTHAIModule/Source/ResourceManager.h"
#include "Utilities/Logger.h"

using namespace bats;
using namespace BWAPI;
using namespace std;

BuildPlanner* BuildPlanner::instance = NULL;
string BuildPlanner::mCurrentPhase = "early";
BuildPlanner::BuildPlanner(){
	BuildOrderFileReader br = BuildOrderFileReader();
	mtransitionGraph = br.readTransitionFile("transition.txt");
	buildOrder = br.readBuildOrder(mCurrentPhase, mtransitionGraph.early, buildOrder);	 // default is early
	mCoreUnitsList = br.getUnitList();
	lastCallFrame = Broodwar->getFrameCount();
}

BuildPlanner::~BuildPlanner(){
	instance = NULL;
}

BuildPlanner* BuildPlanner::getInstance(){
	if (instance == NULL){
		instance = new BuildPlanner();
	}
	return instance;
}

vector<bats::CoreUnit> BuildPlanner::getUnitList(){
	return mCoreUnitsList;
}

void BuildPlanner::switchToPhase(std::string fileName){
	BuildOrderFileReader br = BuildOrderFileReader();
	if(fileName == ""){
		if(BuildPlanner::mCurrentPhase == "early"){
			br.readBuildOrder("mid", mtransitionGraph.mid, buildOrder);
			BuildPlanner::mCurrentPhase = "mid";
			BuildPlanner::mCoreUnitsList = br.getUnitList();
			UnitCreator::getInstance()->switchPhase();
		}
		else if(BuildPlanner::mCurrentPhase == "mid"){
			br.readBuildOrder("late", mtransitionGraph.late, buildOrder);
			BuildPlanner::mCurrentPhase = "late";
			BuildPlanner::mCoreUnitsList = br.getUnitList();
			UnitCreator::getInstance()->switchPhase();
		}
	}
	else if(fileName.length()>0){
		if(BuildPlanner::mCurrentPhase == "early"){
			if(br.readBuildOrder("mid", fileName, buildOrder).size()>0){
				BuildPlanner::mCoreUnitsList = br.getUnitList();
				BuildPlanner::mCurrentPhase = "mid";
				UnitCreator::getInstance()->switchPhase();
			}
			else
				ERROR_MESSAGE(false, "Error loading file " << fileName);
		}
		else if(BuildPlanner::mCurrentPhase == "mid"){
			if(br.readBuildOrder("late", fileName, buildOrder).size()>0){
				BuildPlanner::mCurrentPhase = "late";
				BuildPlanner::mCoreUnitsList = br.getUnitList();
				UnitCreator::getInstance()->switchPhase();
			}
			else
				ERROR_MESSAGE(false, "Error loading file " << fileName);
		}
		else{
			DEBUG_MESSAGE(utilities::LogLevel_Fine, "All transition used");
		}
	}
}

void BuildPlanner::buildingDestroyed(Unit* building){
	if (building->getType() == UnitTypes::Protoss_Pylon)
	{
		return;
	}
	if (building->getType() == UnitTypes::Terran_Supply_Depot)
	{
		return;
	}
	if (building->getType().isAddon())
	{
		return;
	}
	buildOrder.insert(buildOrder.begin(), building->getType());
}

void BuildPlanner::computeActions(){
	//Don't call too often
	int cFrame = Broodwar->getFrameCount();
	if (cFrame - lastCallFrame < 30)
	{
		return;
	}
	lastCallFrame = cFrame;

	if (AgentManager::getInstance()->getNoWorkers() == 0)
	{
		//No workers so cant do anything
		return;
	}

	//Check if we have possible "locked" items in the buildqueue
	for (size_t i = 0; i < buildQueue.size(); i++)
	{
		int elapsed = cFrame - buildQueue.at(i).assignedFrame;
		if (elapsed >= 2000)
		{
			//Reset the build request
			WorkerAgent* worker = (WorkerAgent*)AgentManager::getInstance()->getAgent(buildQueue.at(i).assignedWorkerId);
			if (worker != NULL)
			{
				worker->reset();
			}
			buildOrder.insert(buildOrder.begin(), buildQueue.at(i).toBuild);
			ResourceManager::getInstance()->unlockResources(buildQueue.at(i).toBuild);
			buildQueue.erase(buildQueue.begin() + i);
			return;
		}
	}

	//Check if we can build next building in the build order
	if (buildOrder.size() > 0)
	{
		// @todo check with unitcreator ?
		executeOrder(buildOrder.at(0));
	}

	//Check if we need more supply buildings
	if (isTerran() || isProtoss())
	{
		if (shallBuildSupply())
		{
			buildOrder.insert(buildOrder.begin(), Broodwar->self()->getRace().getSupplyProvider());
		}
	}

	if (!hasResourcesLeft())
	{
		expand(Broodwar->self()->getRace().getCenter());
	}
}

bool BuildPlanner::hasResourcesLeft(){
	int totalMineralsLeft = 0;

	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->getUnitType().isResourceDepot())
		{
			totalMineralsLeft += mineralsNearby(agent->getUnit()->getTilePosition());
		}
	}

	if (totalMineralsLeft <= 1000)
	{
		return false;
	}
	return true;
}

int BuildPlanner::mineralsNearby(TilePosition center){
	int mineralCnt = 0;

	for(set<Unit*>::iterator m = Broodwar->getMinerals().begin(); m != Broodwar->getMinerals().end(); m++)
	{
		if ((*m)->exists())
		{
			double dist = center.getDistance((*m)->getTilePosition());
			if (dist <= 10)
			{
				mineralCnt += (*m)->getResources();			
			}
		}
	}

	return mineralCnt;
}

bool BuildPlanner::shallBuildSupply(){
	UnitType supply = Broodwar->self()->getRace().getSupplyProvider();

	//1. If command center is next in queue, dont build pylon
	/*if (buildOrder.size() > 0)
	{
		if (buildOrder.at(0).isResourceDepot())
		{
			return false;
		}
	}*/

	//2. Check if any building is unpowered (Protoss only)
	if (isProtoss())
	{
		if (buildOrder.size() > 0)
		{
			if (buildOrder.at(0) != UnitTypes::Protoss_Pylon)
			{
				vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
				for (size_t i = 0; i < agents.size(); i++)
				{
					BaseAgent* agent = agents.at(i);
					if (agent->isAlive())
					{
						Unit* cUnit = agent->getUnit();
						if (cUnit->isUnpowered())
						{
							return true;
						}
					}
				}
			}
		}
	}

	//3. Check if we need supplies
	int supplyTotal = Broodwar->self()->supplyTotal() / 2;
	int supplyUsed = Broodwar->self()->supplyUsed() / 2;
	if (supplyTotal - supplyUsed > 8)
	{
		return false;
	}

	if (supplyTotal >= 200)
	{
		//Reached max supply
		return false;
	}

	//4. Check if there is a supply already in the list
	if (nextIsOfType(supply))
	{
		return false;
	}

	//5. Check if we are already building a supply
	if (supplyBeingBuilt())
	{
		return false;
	}

	//Broodwar->printf("Supplies: %d/%d. Adding supply to buildorder", supplyUsed, supplyTotal);

	return true;
}

bool BuildPlanner::supplyBeingBuilt(){
	//Zerg
	if (isZerg())
	{
		if (countInProduction(UnitTypes::Zerg_Overlord) > 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	//Terran and Protoss
	UnitType supply = Broodwar->self()->getRace().getSupplyProvider();

	//1. Check if we are already building a supply
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive())
		{
			if (agent->getUnitType() == supply)
			{
				if (agent->getUnit()->isBeingConstructed())
				{
					//Found one that is being constructed
					return true;
				}
			}
		}
	}

	//2. Check if we have a supply in build queue
	for (size_t i = 0; i < buildQueue.size(); i++)
	{
		if (buildQueue.at(i).toBuild == supply)
		{
			return true;
		}
	}

	return false;
}

void BuildPlanner::lock(int buildOrderIndex, int unitId){
	UnitType type = buildOrder.at(buildOrderIndex);
	buildOrder.erase(buildOrder.begin() + buildOrderIndex);

	BuildQueueItem item;
	item.toBuild = type;
	item.assignedWorkerId = unitId;
	item.assignedFrame = Broodwar->getFrameCount();

	buildQueue.push_back(item);
}

void BuildPlanner::remove(UnitType type){
	for (size_t i = 0; i < buildOrder.size(); i++)
	{
		if (buildOrder.at(i) == type)
		{
			buildOrder.erase(buildOrder.begin() + i);
			return;
		}
	}
}

void BuildPlanner::unlock(UnitType type){
	for (size_t i = 0; i < buildQueue.size(); i++)
	{
		if (buildQueue.at(i).toBuild == type)
		{
			buildQueue.erase(buildQueue.begin() + i);
			return;
		}
	}
}

void BuildPlanner::handleWorkerDestroyed(UnitType type, int workerID){
	for (size_t i = 0; i < buildQueue.size(); i++)
	{
		if (buildQueue.at(i).assignedWorkerId == workerID)
		{
			buildQueue.erase(buildQueue.begin() + i);
			buildOrder.insert(buildOrder.begin(), type);
			ResourceManager::getInstance()->unlockResources(type);
		}
	}
}

bool BuildPlanner::executeMorph(UnitType target, UnitType evolved){
	BaseAgent* agent = AgentManager::getInstance()->getClosestAgent(Broodwar->self()->getStartLocation(), target);
	if (agent != NULL)
	{
		StructureAgent* sAgent = (StructureAgent*)agent;
		if (sAgent->canMorphInto(evolved))
		{
			sAgent->getUnit()->morph(evolved);
			lock(0, sAgent->getUnitID());
			return true;
		}
	}
	else
	{
		//No building available that can do this morph.
		remove(evolved);
	}
	return false;
}

bool BuildPlanner::executeOrder(UnitType type){
	//Max 3 concurrent buildings allowed at the same time
	if (buildQueue.size() >= 3)
	{
		return false;
	}

	//Hold if we are to build a new base
	if (buildQueue.size() > 0)
	{
		if (buildQueue.at(0).toBuild.isResourceDepot())
		{
			return false;
		}
		vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
		for (size_t i = 0; i < agents.size(); i++)
		{
			if (agents.at(i)->getUnitType().isResourceDepot() && agents.at(i)->getUnit()->isBeingConstructed())
			{
				return false;
			}
		}
	}

	if (type.isResourceDepot())
	{
		TilePosition pos = CoverMap::getInstance()->findExpansionSite();
		if (pos == TilePositions::Invalid)
		{
			//No expansion site found.
			if (buildOrder.size() > 0) buildOrder.erase(buildOrder.begin());
			return true;
		}
	}
	if (type.isRefinery())
	{
		TilePosition rSpot = CoverMap::getInstance()->searchRefinerySpot();
		if (rSpot == TilePositions::Invalid)
		{
			//No build spot found
			if (buildOrder.size() > 0) buildOrder.erase(buildOrder.begin());
			return true;
		}
	}
	if (isZerg())
	{
		pair<UnitType, int> builder = type.whatBuilds();
		if (builder.first != UnitTypes::Zerg_Drone)
		{
			//Needs to be morphed
			if (executeMorph(builder.first, type))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	//Check if we have resources
	if (!ResourceManager::getInstance()->hasResources(type)){
		return false;
	}	
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++){
		BaseAgent* agent = agents.at(i);
		if (agent != NULL && agent->isAlive()){
			if (agent->canBuild(type)){
				if (agent->assignToBuild(type)){
					lock(0, agent->getUnitID());
					return true;
				}
				else{
					//Unable to find a build spot. Don't bother checking for all
					//other workers
					handleNoBuildspotFound(type);
					return false;
				}
			}
		}
	}
	return false;
}

bool BuildPlanner::isTerran(){
	if (Broodwar->self()->getRace() == Races::Terran){
		return true;
	}
	return false;
}

bool BuildPlanner::isProtoss(){
	if (Broodwar->self()->getRace() == Races::Protoss){
		return true;
	}
	return false;
}

bool BuildPlanner::isZerg(){
	if (Broodwar->self()->getRace() == Races::Zerg){
		return true;
	}
	return false;
}

void BuildPlanner::addRefinery(){
	UnitType refinery = Broodwar->self()->getRace().getRefinery();

	if (!this->nextIsOfType(refinery)){
		buildOrder.insert(buildOrder.begin(), refinery);
	}
}

void BuildPlanner::commandCenterBuilt(){
	lastCommandCenter = Broodwar->getFrameCount();
}

string BuildPlanner::format(UnitType type){
	string name = type.getName();
	size_t i = name.find(" ");
	string fname = name.substr(i + 1, name.length());
	return fname;
}

void BuildPlanner::printGraphicDebugInfo(){
	size_t max = 4;
	if (buildOrder.size() < 4)
	{
		max = buildOrder.size();
	}

	int line = 1;
	Broodwar->drawTextScreen(5,0,"Buildorder:");
	for (size_t i = 0; i < max; i++)
	{
		Broodwar->drawTextScreen(5,16*line, format(buildOrder.at(i)).c_str());
		line++;
	}

	size_t qmax = 4;
	if (buildQueue.size() < 4)
	{
		qmax = buildQueue.size();
	}

	line = 1;
	Broodwar->drawTextScreen(150,0,"Buildqueue:");
	for (size_t i = 0; i < qmax; i++)
	{
		Broodwar->drawTextScreen(150,16*line, format(buildQueue.at(i).toBuild).c_str());
		line++;
	}
}

void BuildPlanner::handleNoBuildspotFound(UnitType toBuild){
	bool removeOrder = false;
	if (toBuild == UnitTypes::Protoss_Photon_Cannon) removeOrder = true;
	if (toBuild == UnitTypes::Terran_Missile_Turret) removeOrder = true;
	if (toBuild.isAddon()) removeOrder = true;
	if (toBuild == UnitTypes::Zerg_Spore_Colony) removeOrder = true;
	if (toBuild == UnitTypes::Zerg_Sunken_Colony) removeOrder = true;
	if (toBuild.isResourceDepot()) removeOrder = true;
	if (toBuild.isRefinery()) removeOrder = true;

	if (removeOrder)
	{
		remove(toBuild);
	}

	if (!removeOrder)
	{
		if (isProtoss() && !supplyBeingBuilt())
		{
			//Insert a pylon to increase PSI coverage
			if (!nextIsOfType(UnitTypes::Protoss_Pylon))
			{
				buildOrder.insert(buildOrder.begin(), UnitTypes::Protoss_Pylon);
			}
		}
	}
}

bool BuildPlanner::nextIsOfType(UnitType type){
	if (buildOrder.size() == 0)
	{
		return false;
	}
	else
	{
		if (buildOrder.at(0) == type)
		{
			return true;
		}
	}
	return false;
}

bool BuildPlanner::containsType(UnitType type){
	for (size_t i = 0; i < buildOrder.size(); i++)
	{
		if (buildOrder.at(i) == type)
		{
			return true;
		}
	}
	return false;
}

bool BuildPlanner::coveredByDetector(TilePosition pos){
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive())
		{
			UnitType type = agent->getUnitType();
			if (type.isDetector() && type.isBuilding())
			{
				double range = type.sightRange() * 1.5;
				double dist = agent->getUnit()->getPosition().getDistance(Position(pos));
				if (dist <= range)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void BuildPlanner::addBuilding(UnitType type){
	buildOrder.push_back(type);
}

void BuildPlanner::addBuildingFirst(UnitType type){
	buildOrder.insert(buildOrder.begin(), type);
}

void BuildPlanner::expand(UnitType commandCenterUnit){
	if (containsType(commandCenterUnit))
	{
		return;
	}

	TilePosition pos = CoverMap::getInstance()->findExpansionSite();
	if (pos == TilePositions::Invalid)
	{
		//No expansion site found.
		return;
	}

	buildOrder.insert(buildOrder.begin(), commandCenterUnit);
}

bool BuildPlanner::isExpansionAvailable(UnitType commandCenterUnit){
	if (containsType(commandCenterUnit)){
		return false;
	}

	TilePosition pos = CoverMap::getInstance()->findExpansionSite();
	if (pos == TilePositions::Invalid){
		//No expansion site found.
		return false;
	}
	return true;
}

int BuildPlanner::countInProduction(UnitType type){
	int count = 0;
	
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive())
		{
			if (agent->getUnitType().canProduce() && !agent->getUnit()->isBeingConstructed())
			{
				list<UnitType> queue = agent->getUnit()->getTrainingQueue();
				for (list<UnitType>::const_iterator i=queue.begin(); i != queue.end(); i++)
				{
					if ((*i) == type)
					{
						count++;
					}
				}
			}
		}
	}

	if (isZerg())
	{
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->exists())
			{
				if ((*i)->getType() == UnitTypes::Zerg_Egg)
				{
					if ((*i)->getBuildType() == type)
					{
						count++;
						if (type.isTwoUnitsInOneEgg())
							count++;
					}
				}
			}
		}
	}

	return count;
}
