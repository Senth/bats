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
#include "Utilities/String.h"

using namespace bats;
using namespace BWAPI;
using namespace std;

BuildPlanner* BuildPlanner::msInstance = NULL;

const int SUPPLIES_PROVIDED = 16;
const int SUPPLIES_INCREMENT = 3;
const int SUPPLIES_BEGIN = 8;

BuildPlanner::BuildPlanner(){
	mResourceManager = NULL;
	mCoverMap = NULL;
	mAgentManager = NULL;

	mResourceManager = ResourceManager::getInstance();
	mCoverMap = CoverMap::getInstance();
	mAgentManager = AgentManager::getInstance();

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
	bool readdUnit = true;

	if (building->getType() == UnitTypes::Protoss_Pylon) {
		readdUnit = false;
	}
	else if (building->getType() == UnitTypes::Terran_Supply_Depot) {
		readdUnit = false;
	}
	else if (building->getType().isAddon()) {
		/// @todo add addons here later
		readdUnit = false;;
	}

	if (readdUnit) {
		addItemFirst(building->getType());
	}

	
	// Check if the building was upgrading something?
	bool found = false;
	size_t i = 0;
	while(!found && i < mBuildQueue.size()) {
		if (mBuildQueue[i].assignedBuilderId == building->getID()) {
			mBuildQueue[i].assignedBuilderId = -1;
			mBuildQueue[i].assignedFrame = 0;
			addItemFirst(mBuildQueue[i]);
			mBuildQueue.erase(mBuildQueue.begin() + i);
			found = true;
		}
		++i;
	}
}

void BuildPlanner::computeActions(){
	//Don't call too often
	int cFrame = Broodwar->getFrameCount();
	// @todo config variable
	if (cFrame - mLastCallFrame < 30) {
		return;
	}
	mLastCallFrame = cFrame;

	checkUpgradeDone();

	// We can do something even without workers as we have addons and upgrades.
	//if (mAgentManager->getWorkerCount() == 0)
	//{
	//	//No workers so cant do anything
	//	return;
	//}

	//Check if we have possible "locked" buildings in the build queue
	for (size_t i = 0; i < mBuildQueue.size(); i++) {
		if (mBuildQueue[i].type == BuildType_Structure) {
			int elapsed = cFrame - mBuildQueue[i].assignedFrame;
			/// @todo config variable
			if (elapsed >= 700) {
				//Reset the build request
				WorkerAgent* worker = dynamic_cast<WorkerAgent*>(mAgentManager->getAgent(mBuildQueue[i].assignedBuilderId));
				if (worker != NULL) {
					worker->reset();
				}
				addItemFirst(mBuildQueue[i].structure);
				mResourceManager->unlockResources(mBuildQueue[i].structure);
				mBuildQueue.erase(mBuildQueue.begin() + i);
				return;
			}
		}
	}

	//Check if we can build next building in the build order
	if (!mBuildOrder.empty())
	{
		// @todo check with unitcreator ?
		executeOrder(mBuildOrder.front());
	}

	//Check if we need more supply buildings
	if (isTerran() || isProtoss())
	{
		if (shallBuildSupply())
		{
			addItemFirst(Broodwar->self()->getRace().getSupplyProvider());
		}
	}
}

bool BuildPlanner::hasResourcesLeft() const{
	int totalMineralsLeft = 0;

	vector<BaseAgent*> agents = mAgentManager->getAgents();
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


	// Check if any building is unpowered (Protoss only)
	if (isProtoss()) {
		if (!mBuildOrder.empty()) {
			if (mBuildOrder[0].structure != UnitTypes::Protoss_Pylon) {
				vector<BaseAgent*> agents = mAgentManager->getAgents();
				for (size_t i = 0; i < agents.size(); i++) {
					BaseAgent* agent = agents[i];
					if (agent->isAlive()) {
						Unit* cUnit = agent->getUnit();
						if (cUnit->isUnpowered()) {
							return true;
						}
					}
				}
			}
		}
	}

	// Check if there is a supply already in the list
	if (nextIsOfType(supply)) {
		return false;
	}

	// Get current supply difference
	int supplyTotal = Broodwar->self()->supplyTotal();
	int supplyUsed = Broodwar->self()->supplyUsed();
	int supplyDiff = supplyTotal - supplyUsed;

	//Reached max supply
	if (supplyTotal >= 400) {
		return false;
	}

	// Get number of supplies difference we should have before beginning building
	int cUnitProducingStructures = mAgentManager->getUnitProducingStructureCount();
	int supplyDiffShouldHave = SUPPLIES_BEGIN + (cUnitProducingStructures * SUPPLIES_INCREMENT);

	// Decrease with the number of current supplies we have in production
	supplyDiffShouldHave -= getSuppliesBeingBuiltCount() * SUPPLIES_PROVIDED;

	if (supplyDiffShouldHave - supplyDiff <= 0) {
		return false;
	}

	return true;
}

int BuildPlanner::getSuppliesBeingBuiltCount() const {
	//Zerg
	if (isZerg()) {
		return countInProduction(UnitTypes::Zerg_Overlord) > 0;
	}
	
	// Terran and Protoss
	UnitType supplyType = Broodwar->self()->getRace().getSupplyProvider();
	int cSupplies = 0; 

	// Check if we have a supply in build queue
	for (size_t i = 0; i < mBuildQueue.size(); i++) {
		if (mBuildQueue[i].structure == supplyType) {
			++cSupplies;
		}
	}

	// Check agent manager for those under construction
	const vector<BaseAgent*>& agents = mAgentManager->getAgents();
	for (size_t i = 0; i < agents.size(); ++i) {
		if (agents[i]->getUnitType() == supplyType && agents[i]->isBeingBuilt()) {
			++cSupplies;
		}
	}

	return cSupplies;
}

void BuildPlanner::moveToQueue(int buildOrderIndex, int builderId){
	BuildItem item = mBuildOrder[buildOrderIndex];
	mBuildOrder.erase(mBuildOrder.begin() + buildOrderIndex);

	item.assignedBuilderId = builderId;
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
		if (mBuildQueue.at(i).assignedBuilderId == workerID)
		{
			mBuildQueue.erase(mBuildQueue.begin() + i);
			addItemFirst(type);
			mResourceManager->unlockResources(type);
		}
	}
}

bool BuildPlanner::executeMorph(const UnitType& target, const UnitType& evolved){
	BaseAgent* agent = mAgentManager->getClosestAgent(Broodwar->self()->getStartLocation(), target);
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
			// Remove tech upgrade from build order if we have it already
			if (Broodwar->self()->hasResearched(item.tech)) {
				removeFirstOf(item);
			}

			// Find a agent that can build the upgrade
			const vector<BaseAgent*>& agents = mAgentManager->getAgents();
			for (size_t i = 0; i < agents.size(); ++i) {
				if (canResearch(item.tech, agents[i]->getUnit())) {
					bool researchOk = agents[i]->getUnit()->research(item.tech);
					if (researchOk) {
						moveToQueue(0, agents[i]->getUnitID());
					} else {
						DEBUG_MESSAGE(utilities::LogLevel_Warning, "Failed to order research for " <<
							item.tech.getName()
						);
					}
				}
			}
		}
		break;


	case BuildType_Upgrade:
		if (mResourceManager->hasResources(item.upgrade)) {
			// Do we already have this upgrade?
			if (Broodwar->self()->getUpgradeLevel(item.upgrade) >= item.upgradeLevel) {
				removeFirstOf(item);
			}

			// Find a agent that can build the upgrade
			const vector<BaseAgent*>& agents = mAgentManager->getAgents();
			for (size_t i = 0; i < agents.size(); ++i) {
				if (canUpgrade(item.upgrade, agents[i]->getUnit())) {
					bool upgradeOk = agents[i]->getUnit()->upgrade(item.upgrade);
					if (upgradeOk) {
						moveToQueue(0, agents[i]->getUnitID());
					} else {
						DEBUG_MESSAGE(utilities::LogLevel_Warning, "Failed to order research for " <<
							item.upgrade.getName()
						);
					}
				}
			}
		}
		break;


	// Structures
	case BuildType_Structure: {
		//Max 3 concurrent buildings allowed at the same time
		// No need for a limitation
		//if (mBuildQueue.size() >= 3) {
		//	return false;
		//}
		
		/// @todo Handle addons
		if (item.structure.isAddon()) {
			removeFirstOf(item);
			return false;
		}

		//Hold if we are to build a new base
		//if (!mBuildQueue.empty()) {
		//	if (mBuildQueue.at(0).structure.isResourceDepot()) {
		//		return false;
		//	}

		//	vector<BaseAgent*> agents = mAgentManager->getAgents();
		//	for (size_t i = 0; i < agents.size(); i++) {
		//		if (agents.at(i)->getUnitType().isResourceDepot() &&
		//			agents.at(i)->getUnit()->isBeingConstructed())
		//		{
		//			return false;
		//		}
		//	}
		//}

		if (item.structure.isResourceDepot()) {
			TilePosition pos = mCoverMap->findExpansionSite();
			if (pos == TilePositions::Invalid) {
				//No expansion site found.
				if (mBuildOrder.size() > 0) mBuildOrder.erase(mBuildOrder.begin());
				return true;
			}
		}

		else if (item.structure.isRefinery()) {
			TilePosition rSpot = mCoverMap->findBuildSpot(item.structure);
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
		addItemFirst(refinery);
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
			name = item.upgrade.getName() + " " + utilities::string::toString(item.upgradeLevel);
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
	if (mBuildOrder.size() < 4) {
		max = mBuildOrder.size();
	}

	int line = 1;
	Broodwar->drawTextScreen(5,0,"Buildorder:");
	for (size_t i = 0; i < max; i++) {
		Broodwar->drawTextScreen(5,16*line, format(mBuildOrder[i]).c_str());
		line++;
	}

	size_t qmax = 4;
	if (mBuildQueue.size() < 4)
	{
		qmax = mBuildQueue.size();
	}

	line = 1;
	Broodwar->drawTextScreen(150,0,"Buildqueue:");
	for (size_t i = 0; i < qmax; i++) {
		Broodwar->drawTextScreen(150,16*line, format(mBuildQueue[i]).c_str());
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
		if (isProtoss() && getSuppliesBeingBuiltCount() <= 0)
		{
			//Insert a pylon to increase PSI coverage
			if (!nextIsOfType(UnitTypes::Protoss_Pylon))
			{
				addItemFirst(UnitTypes::Protoss_Pylon);
			}
		}
	}
}

bool BuildPlanner::nextIsOfType(const UnitType& type) const {
	if (mBuildOrder.empty()) {
		return false;
	} else {
		UnitType supplyType = Broodwar->self()->getRace().getSupplyProvider();
		// Check first
		if (type == supplyType || mBuildOrder.front().structure != supplyType) {
			return mBuildOrder.front().structure == type;
		}
		// Check second
		else if (mBuildOrder.size() >= 2) {
			return mBuildOrder[1].structure == type;
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
	const vector<BaseAgent*>& agents = AgentManager::getInstance()->getAgents();
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

void BuildPlanner::addItemFirst(const BuildItem& item){
	bool addFirst = false;
	UnitType supplyType = Broodwar->self()->getRace().getSupplyProvider();
	if (item.structure == supplyType || mBuildOrder.empty() || mBuildOrder.front() != supplyType) {
		addFirst = true;
	}

	if (addFirst) {
		mBuildOrder.insert(mBuildOrder.begin(), item);
	}
	// Else add second
	else {
		mBuildOrder.insert(mBuildOrder.begin()+1, item);
	}
}

void BuildPlanner::expand(){
	TilePosition pos = mCoverMap->findExpansionSite();
	if (pos != TilePositions::Invalid)
	{
		addItemFirst(Broodwar->self()->getRace().getCenter());
	}
}

bool BuildPlanner::isExpansionAvailable() const {
	TilePosition pos = mCoverMap->findExpansionSite();
	if (pos == TilePositions::Invalid){
		//No expansion site found.
		return false;
	}
	return true;
}

int BuildPlanner::countInProduction(const UnitType& type) const {
	int count = 0;
	
	vector<BaseAgent*> agents = mAgentManager->getAgents();
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

void BuildPlanner::checkUpgradeDone() {
	vector<BuildItem>::iterator queueIt = mBuildQueue.begin();
	while (queueIt != mBuildQueue.end()) {
		bool erase = false;
		if (queueIt->type == BuildType_Tech) {
			if (Broodwar->self()->hasResearched(queueIt->tech)) {
				erase = true;
			}
		} else if (queueIt->type == BuildType_Upgrade) {
			if (Broodwar->self()->getUpgradeLevel(queueIt->upgrade) >= queueIt->upgradeLevel) {
				erase = true;
			}
		}

		if (erase) {
			queueIt = mBuildQueue.erase(queueIt);
		} else {
			++queueIt;
		}
	}
}