#pragma once

#include <BWAPI.h>

namespace bats{

struct CoreUnit{
	/**
	* Units except buildings
	*/
	BWAPI::UnitType unit;
	/** Quantity of Must Have units are taken as is and quantity for Percentage units is maintained as decimals
	* E.g Marine 70% and Medics 30% are taken as 7 marines and 3 medics
	*/
	int quantity;
	bool mustHave;
};	
struct ProductionQueueItem{
	BWAPI::UnitType unit;
	//Count to maintain no. of units left to produce
	int remainingLeft;
	bool mustHave;
	int quantity;	
};

/**
 * Manages priority order of individual unit creation (except Buildings). All the structures needs UnitCreator's permission to train an unit.
 * Buildings are managed in BuildPlanner
 * Units are managed based on "Must have" and "Percentage" configuration from the user.
 * Buildings must request permission from UnitCreator to train a unit.
 * Maintained as Singleton class and no computation function used
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 */
class UnitCreator{
private:
	/** unlike other version of this member (from BuildPlanner, BuildOrderFileReader), 	
	* this is priotized according to the order it is placed
	*/
	std::vector<bats::CoreUnit> mCoreUnitsList;
	
	/** Queue to maintain the unit production
	* Must have units and percentage units are maintained
	* Priority: must have units has high priority, priority is found by the order of unit placements	
	*/
	std::vector<bats::ProductionQueueItem> mProductionQueue;	
	/** 
	Used for checking building availability
	*/
	static bool canProceedToNextUnit(BWAPI::UnitType unitType);
	
	static int sMustHave;
	/**
	Forcing Singleton usage
	*/
	UnitCreator(void);

protected:
	static UnitCreator* instance;
	void initProductionQueue();
	/**
	* ProductionQueue is updated based on following,	
	* - if all the queue item's remaining units are 0 then they are updated with the percentages (equivalent decimal)
	*/
	void updateProductionQueue();
	static bool compareByPriority(ProductionQueueItem &a, ProductionQueueItem &b);
	
public:
	/**
	Singleton get instance method.
	*/
	static UnitCreator* getInstance();
	static bool sLockForQueue;
	
	void switchPhase();

	virtual ~UnitCreator(void);

	/** Returns the UnitType to be produced according to the priority
	* Allows only StructureAgents to access this method
	* @param builder unit calling the method (to return the unit type producible by this building)
	* @return unit type to produce
	* Handles prioritization of unit production under various circumstances
	*/
	BWAPI::UnitType getNextProducableUnit(BWAPI::Unit* builder);
	
	/** Called on unitDestruction to balance the Percentage Unit and MustHave unit population
	*/
	void updatePopulation(BWAPI::UnitType typeDestroyed);
	/** Prints debug information
	*/
	void printGraphicDebugInfo();
};
}