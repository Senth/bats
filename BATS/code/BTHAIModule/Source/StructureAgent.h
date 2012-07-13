#ifndef __STRUCTUREAGENT_H__
#define __STRUCTUREAGENT_H__

#include "BaseAgent.h"

/** The StructureAgent is the base agent class for all agents handling buildings. If a building is created and no
 * specific agent for that type is found, the building is assigned to a StructureAgent. StructureAgents are typically
 * agents without logic, for example supply depots. To add logic to a building, for example Terran Academy researching
 * stim packs, an agent implementation for that unit type must be created.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class StructureAgent : public BaseAgent {

private:
	
protected:
	bool repairing;
	
	bool canBuildUnit(BWAPI::UnitType type) const;
	bool canEvolveUnit(BWAPI::UnitType type) const;

	BWAPI::TilePosition getNextScanLocation() const;
	std::vector<BWAPI::TilePosition> hasScanned;

public:
	StructureAgent(BWAPI::Unit* mUnit);

	/** Called each update to issue orders. */
	virtual void computeActions();

	/** Used in debug modes to show a line to the agents' goal. */
	virtual void printGraphicDebugInfo() const;

	/** Checks if the agent can morph into the specified type. Zerg only. */
	bool canMorphInto(BWAPI::UnitType type) const;

	/** Sends a number of workers to a newly constructed base. */
	void sendWorkers();
};

#endif
