#include "ChokeHarassSquad.h"
#include "UnitAgent.h"
#include "AgentManager.h"
#include "ExplorationManager.h"
#include "Commander.h"

ChokeHarassSquad::ChokeHarassSquad(int mId, string mName, int mPriority)
{
	this->id = mId;
	this->type = CHOKEHARASS;
	this->moveType = AIR;
	this->name = mName;
	this->priority = mPriority;
	activePriority = priority;
	active = false;
	required = false;
	goal = Broodwar->self()->getStartLocation();
	goalSetFrame = 0;
	currentState = STATE_NOT_SET;
}

bool ChokeHarassSquad::isActive()
{
	return active;
}

void ChokeHarassSquad::defend(TilePosition mGoal)
{
	if (!active)
	{
		setGoal(mGoal);
	}
}

void ChokeHarassSquad::attack(TilePosition mGoal)
{

}

void ChokeHarassSquad::assist(TilePosition mGoal)
{
	if (!active)
	{
		setGoal(mGoal);
	}
}

void ChokeHarassSquad::computeActions()
{
	if (!active)
	{
		if (isFull())
		{
			//Broodwar->printf("Ready at frame %d", Broodwar->getFrameCount());
			active = true;
		}

		if (analyzed)
		{
			TilePosition defSpot = Commander::getInstance()->findChokePoint();
			if (defSpot.x() != -1)
			{
				goal = defSpot;
			}
		}
		return;
	}

	//First, remove dead agents
	for(int i = 0; i < (int)agents.size(); i++)
	{
		if(!agents.at(i)->isAlive())
		{
			agents.erase(agents.begin() + i);
			i--;
		}
	}

	if (active)
	{
		if (activePriority != priority)
		{
			priority = activePriority;
		}

		TilePosition ePos = ExplorationManager::getInstance()->getClosestSpottedBuilding(Broodwar->self()->getStartLocation());
		if (ePos.x() == -1)
		{
			//No enemy building found, check start locations
			TilePosition nGoal = getNextStartLocation();
			if (nGoal.x() >= 0)
			{
				this->goal = nGoal;
				setMemberGoals(goal);
			}
		}
		else
		{
			//Enemy found, guard their chokepoint
			BWTA::Region* eRegion = getRegion(ePos);
			for(set<Chokepoint*>::const_iterator c=eRegion->getChokepoints().begin();c!=eRegion->getChokepoints().end();c++)
			{
				TilePosition nGoal = TilePosition((*c)->getCenter());
				if (nGoal.x() >= 0)
				{
					this->goal = nGoal;
					setMemberGoals(goal);
				}
			}
		}

		checkAttack();
	}
}

void ChokeHarassSquad::printInfo()
{
	string f = "NotFull";
	if (isFull())
	{
		f = "Full";
	}
	string a = "Inactive";
	if (isActive())
	{
		a = "Active";
	}

	Broodwar->printf("[ChokeHarassSquad %d] (%s, %s) Goal: (%d,%d) prio: %d", id, f.c_str(), a.c_str(), goal.x(), goal.y(), priority);
}

void ChokeHarassSquad::clearGoal()
{
	goal = TilePosition(-1, -1);
}

TilePosition ChokeHarassSquad::getGoal()
{
	return goal;
}

bool ChokeHarassSquad::hasGoal()
{
	if (goal.x() < 0 || goal.y() < 0)
	{
		return false;
	}
	return true;
}
