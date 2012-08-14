#pragma once

#include "StructureAgent.h"
#include "BatsModule/include/ResourceDefs.h"

// forward declarations
namespace bats {
	class ResourceCounter;
}

/** The CommandCenterAgent handles Terran Command Center buildings.
 *
 * @section Implemented abilities:
 * - Trains and keeps the number of SCVs (workers) up. Is implemented in levels
 * where the preferred number of SCVs are higher at higher levels, i.e. later in
 * the game.
 * 
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class CommandCenterAgent : public StructureAgent {
public:
	CommandCenterAgent(BWAPI::Unit* mUnit);

	/**
	 * returns the resource group for the expansion the command center
	 * @return expansion's resource group
	 * @todo Move this functionality to StructureMain
	 */
	bats::ResourceGroupCstPtr getResourceGroup() const;

	virtual void computeActions();

protected:
	static bats::ResourceCounter* msResourceCounter;

private:
	bool mHasSentWorkers;
	bats::ResourceGroupCstPtr mResourceGroup;
};