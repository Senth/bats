#ifndef __TRANSPORTAGENT_H__
#define __TRANSPORTAGENT_H__

#include <BWAPI.h>
#include <list>
#include "UnitAgent.h"

/** 
 * The TransportAgent handles transport units (Terran Dropship and Protoss Shuttle).
 * 
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 * Changed entire behavior of the Transport agent, and added more functionality.
 */
class TransportAgent : public UnitAgent {
public:
	/**
	 * Creates a transport agent
	 */
	TransportAgent(BWAPI::Unit* pUnit);

	/**
	 * Sets a unit to load. The commands will always be queued.
	 * @param unit the unit to load
	 * @return true if the agent will be able to load it, false if there is not enough
	 * space for the unit or if the unit type cannot be transported.
	 * @see clearLoadQueue() to reset loading.
	 */
	bool loadUnit(UnitAgent* pUnit);

	/**
	 * Clears the current load queue and will not try to load the units in the unit queue.
	 */
	void clearLoadQueue();

	/**
	 * Sets the transport to unload all units
	 */
	void unloadAll();

	/**
	 * Returns true if the transportation is loading
	 * @return ture if the transportation is loading
	 */
	bool isLoading() const;

	/**
	 * Returns the current load of units.
	 * @param includeQueuedeUnits set this to true if you want to include queued units to
	 * load. Default is false
	 * @return number of free spaces the transportation has.
	 */
	int getFreeLoadSpace(bool includeQueuedUnits = false) const;

	/** 
	 * Called each update to issue orders.
	 */
	void computeActions();

	/**
	 * Checks if the unit can be loaded (does not check if there is enough space)
	 * @param pUnit the unit to check if it can be loaded.
	 * @return true if the unit can be transported.
	 */
	static bool isValidLoadUnit(UnitAgent* pUnit);

private:	
	/**
	 * Update the queue to remove units that either have died or been loaded.
	 */
	void updateQueue();

	int mLoadMax;
	int mLoadQueueSpaces;
	std::vector<UnitAgent*> mLoadQueue;
	bool mUnloading;
};

#endif
