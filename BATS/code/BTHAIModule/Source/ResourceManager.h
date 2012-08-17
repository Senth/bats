#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <BWAPI.h>

struct ResourceLock {
	BWAPI::UnitType unit;
	int mineralCost;
	int gasCost;

	ResourceLock(BWAPI::UnitType mUnit)
	{
		unit = mUnit;
		mineralCost = mUnit.mineralPrice();
		gasCost = mUnit.gasPrice();
	}
};

/** ResourceManager handles the resources and where to spend them. An agent must ask the ResourceManager for permission to build/upgrade/research
 * before issuing the order.
 *
 * The ResourceManager is implemented as a singleton class. Each class that needs to access ResourceManager can request an instance,
 * and all classes shares the same ResourceManager instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ResourceManager {

private:
	static ResourceManager* instance;

	ResourceManager();

	bool hasProductionBuilding();
	/*
	* @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
	*/

	bool isProductionBuildingsIdle();
	
	/* End
	*/
	std::vector<ResourceLock> locks;

	int calcLockedMinerals();
	int calcLockedGas();

public:
	/** Destructor. */
	~ResourceManager();

	/** Returns the instance to the BuildPlanner that is currently used. */
	static ResourceManager* getInstance();

	/** Checks if we need to construct a new worker.
	 * @deprecated bats::SelfClassifier::areExpansionsSaturated() can be used instead. */
	__declspec(deprecated) bool needWorker();

	/** Checks if we have enough resources free to build the specified unit. */
	bool hasResources(BWAPI::UnitType type);

	/** Checks if we have enough resources free for the specified upgrade. */
	bool hasResources(BWAPI::UpgradeType type);

	/** Checks if we have enough resources free for the specified research. */
	bool hasResources(BWAPI::TechType type);

	/** Checks if we have enough resources free. */
	bool hasResources(int neededMinerals, int neededGas);

	/** Locks resources for use. */
	void lockResources(BWAPI::UnitType type);

	/** Unlocks resources for use. */
	void unlockResources(BWAPI::UnitType type);

	/** Shows some debug info on screen. */
	void printInfo();
};

#endif
