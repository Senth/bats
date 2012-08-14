#pragma once
#include "BTHAIModule/Source/structureagent.h"
#include <BWAPI.h>

namespace bats{

/**
 * Manages Terran structure Barracks' task of unit creating eg. Marine, Medic, Firebat
 * Handles user command "transition"
 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
 */
class TerranProductionBuilding : public StructureAgent
{
public:
	/**
	*
	*/
	TerranProductionBuilding(BWAPI::Unit* unit);

	~TerranProductionBuilding();

	/** Called each update to issue orders. Only function overrided from StructureAgent. */
	virtual void computeActions();
};
}

