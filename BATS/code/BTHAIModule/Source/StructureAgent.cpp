#include "StructureAgent.h"
#include "AgentManager.h"
#include "BatsModule/include/BuildPlanner.h"
#include "BatsModule/include/ExplorationManager.h"
#include "BatsModule/include/Config.h"
#include "BatsModule/include/DefenseManager.h"
#include "WorkerAgent.h"
#include "ResourceManager.h"

using namespace BWAPI;
using namespace BWTA;
using namespace std;


StructureAgent::StructureAgent(Unit* mUnit) : BaseAgent(mUnit)
{
	agentType = "StructureAgent";
}

void StructureAgent::printGraphicDebugInfo() const
{
	if (!isAlive()) return;

	if (bats::config::debug::GRAPHICS_VERBOSITY == bats::config::debug::GraphicsVerbosity_Off ||
		bats::config::debug::modules::AGENT_STRUCTURE == false)
	{
		return;
	}


	// Low
	if (bats::config::debug::GRAPHICS_VERBOSITY >= bats::config::debug::GraphicsVerbosity_Low) {
		//Draw "is working" box
		int total = 0;
		int done = 0;
		string txt = "";
		Color cColor = Colors::Blue;
		int bar_h = 18;

		if (unit->isBeingConstructed())
		{
			int w = unit->getType().tileWidth() * 32 / 2;
			Broodwar->drawText(CoordinateType::Map, unit->getPosition().x() - w, unit->getPosition().y() - 10, unit->getType().getName().c_str());
			
			total = unit->getType().buildTime();
			done = total - unit->getRemainingBuildTime();
			txt = "";
			bar_h = 8;
		}
		if (!unit->isBeingConstructed() && unit->getType().isResourceContainer())
		{
			total = unit->getInitialResources();
			done = unit->getResources();
			txt = "";
			cColor = Colors::Orange;
			bar_h = 8;
		}
		if (unit->isResearching())
		{
			total = unit->getTech().researchTime();
			done = total - unit->getRemainingResearchTime();
			txt = unit->getTech().getName();
		}
		if (unit->isUpgrading())
		{
			total = unit->getUpgrade().upgradeTime();
			done = total - unit->getRemainingUpgradeTime();
			txt = unit->getUpgrade().getName();
		}
		if (unit->isTraining())
		{
			if (unit->getTrainingQueue().size() > 0)
			{
				UnitType t = *(unit->getTrainingQueue().begin());
				total = t.buildTime();
				txt = t.getName();
				done = total - unit->getRemainingTrainTime();
			}
		}

		if (total > 0)
		{
			int w = unit->getType().tileWidth() * 32;
			int h = unit->getType().tileHeight() * 32;

			//Start 
			Position s = Position(unit->getPosition().x() - w/2, unit->getPosition().y() - 30);
			//End
			Position e = Position(s.x() + w, s.y() + bar_h);
			//Progress
			int prg = (int)((double)done / (double)total * w);
			Position p = Position(s.x() + prg, s.y() +  bar_h);

			Broodwar->drawBox(CoordinateType::Map,s.x(),s.y(),e.x(),e.y(),cColor,false);
			Broodwar->drawBox(CoordinateType::Map,s.x(),s.y(),p.x(),p.y(),cColor,true);

			Broodwar->drawText(CoordinateType::Map, s.x() + 5, s.y() + 2, txt.c_str());
		}

		if (!unit->isBeingConstructed() && unit->getType().getID() == UnitTypes::Terran_Bunker.getID())
		{
			int w = unit->getType().tileWidth() * 32;
			int h = unit->getType().tileHeight() * 32;

			Broodwar->drawText(CoordinateType::Map, unit->getPosition().x() - w / 2, unit->getPosition().y() - 10, unit->getType().getName().c_str());

			//Draw Loaded box
			Position a = Position(unit->getPosition().x() - w/2, unit->getPosition().y() - h/2);
			Position b = Position(a.x() + 94, a.y() + 6);
			Broodwar->drawBox(CoordinateType::Map,a.x(),a.y(),b.x(),b.y(),Colors::Green,false);

			if ((int)unit->getLoadedUnits().size() > 0)
			{
				Position a = Position(unit->getPosition().x() - w/2, unit->getPosition().y() - h/2);
				Position b = Position(a.x() + unit->getLoadedUnits().size() * 24, a.y() + 6);

				Broodwar->drawBox(CoordinateType::Map,a.x(),a.y(),b.x(),b.y(),Colors::Green,true);
			}
		}
	}
}

void StructureAgent::computeActions()
{
	if (isAlive())
	{
		handleUnderAttack();

		if (!unit->isIdle())
		{
			return;
		}

		if (bats::BuildPlanner::isTerran())
		{
			//Check addons here
			if (isOfType(UnitTypes::Terran_Science_Facility))
			{
				if (unit->getAddon() == NULL)
				{
					unit->buildAddon(UnitTypes::Terran_Physics_Lab);
				}
			}
			if (isOfType(UnitTypes::Terran_Starport))
			{
				if (unit->getAddon() == NULL)
				{
					unit->buildAddon(UnitTypes::Terran_Control_Tower);
				}
			}			
			if (isOfType(UnitTypes::Terran_Factory))
			{
				if (unit->getAddon() == NULL)
				{
					unit->buildAddon(UnitTypes::Terran_Machine_Shop);
				}
			}
		}

		// This should not be here, UnitCreator handles building units!
		//if (!unit->isBeingConstructed() && unit->isIdle() && getUnit()->getType().canProduce())
		//{
		//	//Iterate through all unit types
		//	for(set<UnitType>::const_iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end();i++)
		//	{
		//		//Check if we can (and need) to build the unit
		//		if (canBuildUnit(*i))
		//		{
		//			//Build it!
		//			unit->train(*i);
		//		}
		//	}
		//}
	}
}

bool StructureAgent::canBuildUnit(const UnitType& type) const
{
	//1. Check if race matches
	if (type.getRace() != Broodwar->self()->getRace())
	{
		return false;
	}

	//2. Check if this unit can construct the unit
	pair<UnitType, int> builder = type.whatBuilds();
	if (builder.first != unit->getType())
	{
		return false;
	}

	/*3. Check if we need the unit in a squad
	if (!Commander::getInstance()->needUnit(type))
	{
		return false;
	}
	*/

	//4. Check canBuild
	if (!Broodwar->canMake(unit, type))
	{			
		return false;
	}

	// 5. Check if we have enough resources
	if (!ResourceManager::getInstance()->hasResources(type))
	{
		return false;
	}
	

	//All clear. Build the unit.
	return true;
}

void StructureAgent::sendWorkers()
{
	//We have constructed a new base. Make some workers move here.
	int noWorkers = AgentManager::getInstance()->getWorkerCount();
	int toSend = noWorkers / AgentManager::getInstance()->countNoBases();
	int hasSent = 0;

	//Broodwar->printf("Sending %d/%d workers to new base", toSend, noWorkers);
	vector<BaseAgent*> agents = AgentManager::getInstance()->getAgents();
	for (int i = 0; i < (int)agents.size(); i++)
	{
		BaseAgent* agent = agents.at(i);
		if (agent->isAlive() && agent->isFreeWorker())
		{
			Unit* worker = agent->getUnit();
			WorkerAgent* wa = (WorkerAgent*)agent;
			worker->rightClick(unit->getPosition());
			hasSent++;
		}

		if (hasSent >= toSend)
		{
			return;
		}
	}
}

bool StructureAgent::canMorphInto(const UnitType& type) const
{
	//1. Check canBuild
	if (!Broodwar->canMake(unit, type))
	{
		return false;
	}
	
	//2. Check if we have enough resources
	if (!ResourceManager::getInstance()->hasResources(type))
	{
		return false;
	}

	//3. Check if unit is already morphing
	if (unit->isMorphing())
	{
		return false;
	}

	//All clear. Build the unit.
	return true;
}

bool StructureAgent::canEvolveUnit(const UnitType& type) const
{
	//1. Check if we need the unit in a squad
	//if (!Commander::getInstance()->needUnit(type))
	//{
	//	return false;
	//}

	//2. Check canBuild
	if (!Broodwar->canMake(unit, type))
	{
		return false;
	}

	//3. Check if we have enough resources
	if (!ResourceManager::getInstance()->hasResources(type))
	{
		return false;
	}

	//All clear. Build the unit.
	return true;
}

void StructureAgent::handleUnderAttack() {
	// Report to defense manager
	if (unit->isUnderAttack()) {
		msDefenseManager->onStructureUnderAttack(this);
	}
}