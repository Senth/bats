#include "BuildPlanner.h"
#include "UnitManager.h"
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

BuildPlanner* BuildPlanner::msInstance = NULL;

BuildPlanner::BuildPlanner(){
	mResourceManager = NULL;

	mResourceManager = ResourceManager::getInstance();

	mCurrentPhase = "early";
	BuildOrderFileReader br = BuildOrderFileReader();
	mtransitionGraph = br.readTransitionFile("transition.txt");
	br.readBuildOrder(mCurrentPhase, mtransitionGraph.early, mBuildOrder);	 // default is early
	mCoreUnitsList = br.getUnitList();
	mLastCallFrame = Broodwar->getFrameCount();
}

BuildPlanner::~BuildPlanner(){
	msInstance = NULL;
}

BuildPlanner* BuildPlanner::getInstance(){
	if (msInstance == NULL){
		msInstance = new BuildPlanner();
	}
	return msInstance;
}

size_t BuildPlanner::getQueueCount() const {
	return mBuildOrder.size();
}

vector<bats::CoreUnit> BuildPlanner::getUnitList() const{
	return mCoreUnitsList;
}

const string& BuildPlanner::getCurrentPhase() const {
	return mCurrentPhase;
}

bool BuildPlanner::canTransition() const {
	return mCurrentPhase != "late";
}

void BuildPlanner::switchToPhase(const std::string& fileName){
	BuildOrderFileReader br;
	if(fileName.empty()){
		if(mCurrentPhase == "early"){
			br.readBuildOrder("mid", mtransitionGraph.mid, mBuildOrder);
			mCurrentPhase = "mid";
			mCoreUnitsList = br.getUnitList();
			UnitCreator::getInstance()->switchPhase();
		}
		else if(mCurrentPhase == "mid"){
			br.readBuildOrder("late", mtransitionGraph.late, mBuildOrder);
			mCurrentPhase = "late";
			mCoreUnitsList = br.getUnitList();
			UnitCreator::getInstance()->switchPhase();
		}
	}

	else {
		if(mCurrentPhase == "early"){
			br.readBuildOrder("mid", fileName, mBuildOrder);
			mCoreUnitsList = br.getUnitList();
			mCurrentPhase = "mid";
			UnitCreator::getInstance()->switchPhase();
		}
		else if(mCurrentPhase == "mid"){
			br.readBuildOrder("late", fileName, mBuildOrder);
			mCurrentPhase = "late";
			mCoreUnitsList = br.getUnitList();
			UnitCreator::getInstance()->switchPhase();
		}
		else{
			DEBUG_MESSAGE(utilities::LogLevel_Fine, "All transition used");
		}
	}
}

void BuildPlanner::buildingDestroyed(const Unit* building){
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
		/// @todo add addons here later
		return;
	}
	addBuildingFirst(building->getType());
}

void BuildPlanner::computeActions(){
	//Don't call too often
	int cFrame = Broodwar->getFrameCount();
	if (cFrame - mLastCallFrame < 30)
	{
		return;
	}
	mLastCallFrame = cFrame;

	if (AgentManager::getInstance()->getWorkerCount() == 0)
	{
		//No workers so cant do anything
		return;
	}

	//Check if we have possible "locked" items in the build queue
	for (size_t i = 0; i < mBuildQueue.size(); i++)
	{
		int elapsed = cFrame - mBuildQueue.at(i).assignedFrame;
		if (elapsed >= 2000)
		{
			//Reset the build request
			WorkerAgent* worker = (WorkerAgent*)AgentManager::getInstance()->getAgent(mBuildQueue.at(i).assignedBuildId);
			if (worker != NULL)
			{
				worker->reset();
			}
			mBuildOrder.insert(mBuildOrder.begin(), mBuildQueue.at(i).structure);
			mResourceManager->unlockResources(mBuildQueue.at(i).structure);
			mBuildQueue.erase(mBuildQueue.begin() + i);
			return;
		}
	}

	//Check if we can build next building in the build order
	if (mBuildOrder.size() > 0)
	{
		// @todo check with unitcreator ?
		executeOrder(mBuildOrder.at(0));
	}

	//Check if we need more supply buildings
	if (isTerran() || isProtoss())
	{
		if (shallBuildSupply())
		{
			mBuildOrder.insert(mBuildOrder.begin(), Broodwar->self()->getRace().getSupplyProvider());
		}
	}

	// Commander checks if we should expand
	//if (!hasResourcesLeft())
	//{
	//	expand();
	//}
}

bool BuildPlanner::hasResourcesLeft() const{
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

int BuildPlanner::mineralsNearby(const TilePosition& center) const{
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

bool BuildPlanner::shallBuildSupply() const{
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
		if (!mBuildOrder.empty())
		{
			if (mBuildOrder[0].structure != UnitTypes::Protoss_Pylon)
			{
				vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
				for (size_t i = 0; i < agents.size(); i++)
				{
					BaseAgent* agent = agents[i];
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

bool BuildPlanner::supplyBeingBuilt() const{
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

	//1. Check if we have a supply in build queue
	for (size_t i = 0; i < mBuildQueue.size(); i++)
	{
		if (mBuildQueue[i].structure == supply)
		{
			return true;
		}
	}

	//2. Check if we are already building a supply
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (size_t i = 0; i < agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive())
		{
			if (agent->getUnitType() == supply)
			{
				if (!agent->getUnit()->isCompleted())
				{
					//Found one that is being constructed
					return true;
				}
			}
		}
	}

	return false;
}

void BuildPlanner::moveToQueue(int buildOrderIndex, int builderId){
	BuildItem item = mBuildOrder.at(buildOrderIndex);
	mBuildOrder.erase(mBuildOrder.begin() + buildOrderIndex);

	item.assignedBuildId = builderId;
	item.assignedFrame = Broodwar->getFrameCount();

	mBuildQueue.push_back(item);
}

void BuildPlanner::removeFirstOf(const UnitType& type){
	for (size_t i = 0; i < mBuildOrder.size(); i++) {
		if (mBuildOrder.at(i).structure == type) {
			mBuildOrder.erase(mBuildOrder.begin() + i);
			return;
		}
	}
}

void BuildPlanner::removeFirstOf(const BuildItem& item) {
	for (size_t i = 0; i < mBuildOrder.size(); i++) {
		if (mBuildOrder[i] == item) {
			mBuildOrder.erase(mBuildOrder.begin() + i);
			return;
		}
	}
}

void BuildPlanner::removeFromQueue(const UnitType& type){
	for (size_t i = 0; i < mBuildQueue.size(); i++) {
		if (mBuildQueue.at(i).structure == type) {
			mBuildQueue.erase(mBuildQueue.begin() + i);
			return;
		}
	}
}

void BuildPlanner::handleWorkerDestroyed(const UnitType& type, int workerID){
	for (size_t i = 0; i < mBuildQueue.size(); i++)
	{
		if (mBuildQueue.at(i).assignedBuildId == workerID)
		{
			mBuildQueue.erase(mBuildQueue.begin() + i);
			mBuildOrder.insert(mBuildOrder.begin(), type);
			mResourceManager->unlockResources(type);
		}
	}
}

bool BuildPlanner::executeMorph(const UnitType& target, const UnitType& evolved){
	BaseAgent* agent = AgentManager::getInstance()->getClosestAgent(Broodwar->self()->getStartLocation(), target);
	if (agent != NULL)
	{
		StructureAgent* sAgent = (StructureAgent*)agent;
		if (sAgent->canMorphInto(evolved))
		{
			sAgent->getUnit()->morph(evolved);
			moveToQueue(0, sAgent->getUnitID());
			return true;
		}
	}
	else
	{
		//No building available that can do this morph.
		removeFirstOf(evolved);
	}
	return false;
}

bool BuildPlanner::executeOrder(const BuildItem& item){
	switch (item.type) {
	case BuildType_Tech:
		if (mResourceManager->hasResources(item.tech)) {
			// Do we already have this tech upgrade?
			if (Broodwar->self()->hasResearched(item.tech)) {
				removeFirstOf(item);
			}

			const vector<BaseAgent*>& agents = AgentManager::getInstance()->getAgents();
			for (size_t i = 0; i < agents.size(); ++i) {
				
			}
		}
		break;


	case BuildType_Upgrade:
		if (mResourceManager->hasResources(item.upgrade)) {
			// Do we already have this upgrade?
			if (Broodwar->self()->getUpgradeLevel(item.upgrade) <= item.upgradeLevel) {
				removeFirstOf(item);
			}

			/// @todo
		}
		break;


	// Structures
	case BuildType_Structure: {
		//Max 3 concurrent buildings allowed at the same time
		// No need for a limitation
		//if (mBuildQueue.size() >= 3) {
		//	return false;
		//}

		//Hold if we are to build a new base
		if (!mBuildQueue.empty()) {
			if (mBuildQueue.at(0).structure.isResourceDepot()) {
				return false;
			}

			vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
			for (size_t i = 0; i < agents.size(); i++) {
				if (agents.at(i)->getUnitType().isResourceDepot() &&
					agents.at(i)->getUnit()->isBeingConstructed())
				{
					return false;
				}
			}
		}

		else if (item.structure.isResourceDepot()) {
			TilePosition pos = CoverMap::getInstance()->findExpansionSite();
			if (pos == TilePositions::Invalid) {
				//No expansion site found.
				if (mBuildOrder.size() > 0) mBuildOrder.erase(mBuildOrder.begin());
				return true;
			}
		}

		else if (item.structure.isRefinery()) {
			TilePosition rSpot = CoverMap::getInstance()->searchRefinerySpot();
			if (rSpot == TilePositions::Invalid) {
				//No build spot found
				if (mBuildOrder.size() > 0) mBuildOrder.erase(mBuildOrder.begin());
				return true;
			}
		}

		if (isZerg()) {
			pair<UnitType, int> builder = item.structure.whatBuilds();
			if (builder.first != UnitTypes::Zerg_Drone) {
				//Needs to be morphed
				if (executeMorph(builder.first, item.structure)) {
					return true;
				}
				else {
					return false;
				}
			}
		}

		//Check if we have resources
		if (!mResourceManager->hasResources(item.structure)){
			return false;
		}

		vector<UnitAgent*> freeWorkers = UnitManager::getInstance()->getUnitsByFilter(UnitFilter_WorkersFree);
		for (size_t i = 0; i < freeWorkers.size(); i++){
			UnitAgent* agent = freeWorkers.at(i);
			if (agent != NULL && agent->isAlive()){
				if (agent->canBuild(item.structure)){
					if (agent->assignToBuild(item.structure)){
						moveToQueue(0, agent->getUnitID());
						return true;
					}
					else{
						//Unable to find a build spot. Don't bother checking for all
						//other workers
						handleNoBuildspotFound(item.structure);
						return false;
					}
				}
			}
		}
		return false;
		break;
	}

	default:
		ERROR_MESSAGE(false, "BuildPlanner: Unknown build type");
		break;

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
		mBuildOrder.insert(mBuildOrder.begin(), refinery);
	}
}

string BuildPlanner::format(const BuildItem& item) const{
	string name = "Unknown";
	switch (item.type) {
		case BuildType_Structure:
			name = item.structure.getName();
			break;

		case BuildType_Tech:
			name = item.tech.getName();
			break;

		case BuildType_Upgrade:
			name = item.upgrade.getName();
			break;

		default:
			name = "Unknown";
			break;
	}
	size_t i = name.find(" ");
	name = name.substr(i + 1);
	return name;
}

void BuildPlanner::printGraphicDebugInfo() const {
	/// @todo add debug module and config from config.h
	size_t max = 4;
	if (mBuildOrder.size() < 4)
	{
		max = mBuildOrder.size();
	}

	int line = 1;
	Broodwar->drawTextScreen(5,0,"Buildorder:");
	for (size_t i = 0; i < max; i++)
	{
		Broodwar->drawTextScreen(5,16*line, format(mBuildOrder.at(i)).c_str());
		line++;
	}

	size_t qmax = 4;
	if (mBuildQueue.size() < 4)
	{
		qmax = mBuildQueue.size();
	}

	line = 1;
	Broodwar->drawTextScreen(150,0,"Buildqueue:");
	for (size_t i = 0; i < qmax; i++)
	{
		Broodwar->drawTextScreen(150,16*line, format(mBuildQueue.at(i).structure).c_str());
		line++;
	}
}

void BuildPlanner::handleNoBuildspotFound(const UnitType& toBuild){
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
		removeFirstOf(toBuild);
	}

	if (!removeOrder)
	{
		if (isProtoss() && !supplyBeingBuilt())
		{
			//Insert a pylon to increase PSI coverage
			if (!nextIsOfType(UnitTypes::Protoss_Pylon))
			{
				mBuildOrder.insert(mBuildOrder.begin(), UnitTypes::Protoss_Pylon);
			}
		}
	}
}

bool BuildPlanner::nextIsOfType(const UnitType& type) const {
	if (mBuildOrder.empty())
	{
		return false;
	}
	else
	{
		if (mBuildOrder[0].structure == type)
		{
			return true;
		}
	}
	return false;
}

bool BuildPlanner::containsType(const UnitType& type) const {
	for (size_t i = 0; i < mBuildOrder.size(); i++)
	{
		if (mBuildOrder[i].structure == type)
		{
			return true;
		}
	}
	return false;
}

bool BuildPlanner::coveredByDetector(const TilePosition& pos) {
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

void BuildPlanner::addBuilding(const UnitType& type){
	mBuildOrder.push_back(type);
}

void BuildPlanner::addBuildingFirst(const UnitType& type){
	mBuildOrder.insert(mBuildOrder.begin(), type);
}

void BuildPlanner::expand(){
	TilePosition pos = CoverMap::getInstance()->findExpansionSite();
	if (pos != TilePositions::Invalid)
	{
		mBuildOrder.insert(mBuildOrder.begin(), Broodwar->self()->getRace().getCenter());
	}
}

bool BuildPlanner::isExpansionAvailable() const {
	TilePosition pos = CoverMap::getInstance()->findExpansionSite();
	if (pos == TilePositions::Invalid){
		//No expansion site found.
		return false;
	}
	return true;
}

int BuildPlanner::countInProduction(const UnitType& type) const {
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

bool BuildPlanner::canUpgrade(const UpgradeType& type, const Unit* unit) const {
	//1. Check if unit is idle
	if (!unit->isIdle()) {
		return false;
	}

	//2. Check if unit can do this upgrade
	if (!Broodwar->canUpgrade(unit, type)) {
		return false;
	}

	//3. Check if we have enough resources
	if (!ResourceManager::getInstance()->hasResources(type)) {
		return false;
	}

	//4. Check if unit is being constructed
	if (unit->isBeingConstructed()) {
		return false;
	}

	//5. Check if we are currently upgrading it
	if (Broodwar->self()->isUpgrading(type)) {
		return false;
	}

	//All clear. Can do the upgrade.
	return true;
}

bool BuildPlanner::canResearch(const TechType& type, const Unit* unit) const {
	//1. Check if unit can do this upgrade
	if (!Broodwar->canResearch(unit, type)) {
		return false;
	}

	//2. Check if we have enough resources
	if (!ResourceManager::getInstance()->hasResources(type)) {
		return false;
	}

	//3. Check if unit is idle
	if (!unit->isIdle()) {
		return false;
	}

	//4. Check if unit is being constructed
	if (unit->isBeingConstructed()) {
		return false;
	}

	//5. Check if we are currently researching it
	if (Broodwar->self()->isResearching(type)) {
		return false;
	}

	//All clear. Can do the research.
	return true;
}
