#include "AgentFactory.h"
#include "WorkerAgent.h"
#include "StructureAgent.h"
#include "UnitAgent.h"
#include "TransportAgent.h"
#include "BATSModule/include/SquadDefs.h"

/** Terran agents */
#include "CommandCenterAgent.h"
#include "ComsatAgent.h"
#include "SiegeTankAgent.h"
#include "MarineAgent.h"
#include "MedicAgent.h"
#include "VultureAgent.h"
#include "FirebatAgent.h"
#include "RefineryAgent.h"
#include "ScienceVesselAgent.h"
#include "BattlecruiserAgent.h"
#include "WraithAgent.h"
#include "GhostAgent.h"
#include "GoliathAgent.h"
#include "ValkyrieAgent.h"
#include "BatsModule/include/TerranProductionBuilding.h"

/** Protoss agents */
#include "NexusAgent.h"
#include "ZealotAgent.h"
#include "DragoonAgent.h"
#include "ReaverAgent.h"
#include "ObserverAgent.h"
#include "DarkTemplarAgent.h"
#include "ScoutAgent.h"
#include "CorsairAgent.h"
#include "CarrierAgent.h"
#include "HighTemplarAgent.h"
#include "ArbiterAgent.h"

/** Zerg agents */
#include "HatcheryAgent.h"
#include "OverlordAgent.h"
#include "ZerglingAgent.h"
#include "HydraliskAgent.h"
#include "LurkerAgent.h"
#include "MutaliskAgent.h"
#include "QueenAgent.h"
#include "UltraliskAgent.h"
#include "DevourerAgent.h"
#include "GuardianAgent.h"
#include "DefilerAgent.h"
#include "ScourgeAgent.h"
#include "InfestedTerranAgent.h"

using namespace BWAPI;
using namespace std;

bool AgentFactory::instanceFlag = false;
AgentFactory* AgentFactory::instance = NULL;

AgentFactory::AgentFactory()
{
	
}

AgentFactory::~AgentFactory()
{
	instanceFlag = false;
	delete instance;
}

AgentFactory* AgentFactory::getInstance()
{
	if (!instanceFlag)
	{
		instance = new AgentFactory();
		instanceFlag = true;
	}
	return instance;
}

BaseAgent* AgentFactory::createAgent(Unit* unit)
{
	BaseAgent* pNewAgent = NULL;

	if (Broodwar->self()->getRace() == Races::Terran)
	{
		pNewAgent = createTerranAgent(unit);
	}
	else if (Broodwar->self()->getRace() == Races::Protoss)
	{
		pNewAgent = createProtossAgent(unit);
	}
	else if (Broodwar->self()->getRace() == Races::Zerg)
	{
		pNewAgent = createZergAgent(unit);
	}

	//Default agents
	if (NULL == pNewAgent) {
		if (unit->getType().isWorker())
		{
			pNewAgent = new WorkerAgent(unit);
		}
		else if (unit->getType().isBuilding()){		
			pNewAgent = new StructureAgent(unit);
		}
		else 
		{
			pNewAgent = new UnitAgent(unit);
		}
	}
	

	return pNewAgent;
}

BaseAgent* AgentFactory::createZergAgent(Unit* unit)
{
	UnitType type = unit->getType();
	
	if (type.isWorker())
	{
		return new WorkerAgent(unit);
	}
	else if (type.isBuilding())
	{
		//Add agents for special buildings here
		if (type == UnitTypes::Zerg_Hatchery)
		{
			return new HatcheryAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Lair)
		{
			return new HatcheryAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Hive)
		{
			return new HatcheryAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Extractor)
		{
			return new RefineryAgent(unit);
		}
		else
		{
			//Default structure agent
			return new StructureAgent(unit);
		}
	}
	else
	{
#if DISABLE_UNIT_AI == 0 && !defined(DISABLE_ZERG_UNITS)
		if (type == UnitTypes::Zerg_Overlord)
		{
			return new OverlordAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Zergling)
		{
			return new ZerglingAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Hydralisk)
		{
			return new HydraliskAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Mutalisk)
		{
			return new MutaliskAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Lurker)
		{
			return new LurkerAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Queen)
		{
			return new QueenAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Ultralisk)
		{
			return new UltraliskAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Guardian)
		{
			return new GuardianAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Devourer)
		{
			return new DevourerAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Defiler)
		{
			return new DefilerAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Scourge)
		{
			return new ScourgeAgent(unit);
		}
		else if (type == UnitTypes::Zerg_Infested_Terran)
		{
			return new InfestedTerranAgent(unit);
		}
		else
		{
			//Default unit agent
			return new UnitAgent(unit);
		}
#else
		return new UnitAgent(unit);
#endif
	}
	return NULL;
}

BaseAgent* AgentFactory::createTerranAgent(Unit* unit)
{
	if (unit->getType().isWorker())
	{
		return new WorkerAgent(unit);
	}
	else if (unit->getType().isBuilding())
	{
		//Add agents for special buildings here
		if (isOfType(unit, UnitTypes::Terran_Command_Center))
		{
			return new CommandCenterAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Comsat_Station))
		{
			return new ComsatAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Refinery))
		{
			return new RefineryAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Bunker))
		{
			/// @todo how to use bunkers?
			//Commander::getInstance()->addBunkerSquad();
			return new StructureAgent(unit);
		}
		/**
		 * Subclass of Structure agent for Terran production buildings
		 * @author Suresh K. Balsasubramaniyan (suresh.draco@gmail.com)
		 */
		else if(unit->getType() == UnitTypes::Terran_Barracks || 
			unit->getType() == UnitTypes::Terran_Factory || unit->getType() == UnitTypes::Terran_Starport)
			return new bats::TerranProductionBuilding(unit);
		else
		{
			//Default structure agent
			return new StructureAgent(unit);
		}
	}
	else
	{
#if DISABLE_UNIT_AI == 0 && !defined(DISABLE_TERRAN_UNITS)
		if (isOfType(unit, UnitTypes::Terran_Siege_Tank_Tank_Mode))
		{
			return new SiegeTankAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Siege_Tank_Siege_Mode))
		{
			return new SiegeTankAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Marine))
		{
			return new MarineAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Firebat))
		{
			return new FirebatAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Medic))
		{
			return new MedicAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Vulture))
		{
			return new VultureAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Science_Vessel))
		{
			return new ScienceVesselAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Battlecruiser))
		{
			return new BattlecruiserAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Wraith))
		{
			return new WraithAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Ghost))
		{
			return new GhostAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Goliath))
		{
			return new GoliathAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Valkyrie))
		{
			return new ValkyrieAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Terran_Dropship))
		{
			return new TransportAgent(unit);
		}
		else
		{
			//Default unit agent
			return new UnitAgent(unit);
		}
#else
		return new UnitAgent(unit);
#endif
	}
	return NULL;
}

BaseAgent* AgentFactory::createProtossAgent(Unit* unit)
{
	if (unit->getType().isWorker())
	{
		return new WorkerAgent(unit);
	}
	else if (unit->getType().isBuilding())
	{
		//Add agents for special buildings here
		if (isOfType(unit, UnitTypes::Protoss_Nexus))
		{
			return new NexusAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Assimilator))
		{
			return new RefineryAgent(unit);
		}
		else
		{
			//Default structure agent
			return new StructureAgent(unit);
		}
	}
	else
	{
#if DISABLE_UNIT_AI == 0 && !defined(DISABLE_PROTOSS_UNITS)
		if (isOfType(unit, UnitTypes::Protoss_Zealot))
		{
			return new ZealotAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Dragoon))
		{
			return new DragoonAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Reaver))
		{
			return new ReaverAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Observer))
		{
			return new ObserverAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Dark_Templar))
		{
			return new DarkTemplarAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Scout))
		{
			return new ScoutAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Shuttle))
		{
			return new TransportAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Corsair))
		{
			return new CorsairAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Carrier))
		{
			return new CarrierAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_High_Templar))
		{
			return new HighTemplarAgent(unit);
		}
		else if (isOfType(unit, UnitTypes::Protoss_Arbiter))
		{
			return new ArbiterAgent(unit);
		}
		else
		{
			//Default unit agent
			return new UnitAgent(unit);
		}
#else
		return new UnitAgent(unit);
#endif
	}
	return NULL;
}

bool AgentFactory::isOfType(Unit* unit, UnitType type)
{
	if (unit->getType() == type)
	{
		return true;
	}
	return false;
}

