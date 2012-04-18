#include "RushSquad.h"
#include "UnitAgent.h"
#include "AgentManager.h"
#include "ExplorationManager.h"
#include "Commander.h"

using namespace BWAPI;
using namespace BWTA;
using namespace std;

RushSquad::RushSquad(int mId, string mName, int mPriority)
{
	this->id = mId;
	this->type = RUSH;
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

bool RushSquad::isActive()
{
	return active;
}

void RushSquad::defend(TilePosition mGoal)
{
	if (!active)
	{
		setGoal(mGoal);
	}
}

void RushSquad::attack(TilePosition mGoal)
{

}

void RushSquad::assist(TilePosition mGoal)
{
	if (!active)
	{
		setGoal(mGoal);
	}
}

void RushSquad::computeActions()
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
			if (defSpot!= TilePositions::Invalid)
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

		Unit* target = findWorkerTarget();
		if (target != NULL)
		{
			for (int i = 0; i < (int)agents.size(); i++)
			{
				BaseAgent* agent = agents.at(i);
				if (agent->isAlive())
				{
					agent->getUnit()->attack(target);
				}
			}
		}

		TilePosition ePos = ExplorationManager::getInstance()->getClosestSpottedBuilding(Broodwar->self()->getStartLocation());
		if (ePos== TilePositions::Invalid)
		{
			TilePosition nGoal = getNextStartLocation();
			if (nGoal != TilePositions::Invalid)
			{
				this->goal = nGoal;
				setMemberGoals(goal);
			}
		}
		else
		{
			this->goal = ePos;//getClosestStartLocation(ePos);
			setMemberGoals(goal);
		}

		checkAttack();
	}
}

Unit* RushSquad::findWorkerTarget()
{
	try {
		double maxRange = 12 * 32;

		for (int i = 0; i < (int)agents.size(); i++)
		{
			BaseAgent* agent = agents.at(i);

			for(set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++)
			{
				if ((*i)->exists())
				{
					if ((*i)->getType().isWorker())
				{
						double dist = agent->getUnit()->getDistance((*i));
						if (dist <= maxRange)
				{
							return (*i);
						}
					}	
				}
			}
		}
	}
	catch (exception)
	{

	}

	return NULL;
}

void RushSquad::printInfo()
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

	Broodwar->printf("[RushSquad %d] (%s, %s) Goal: (%d,%d) prio: %d", id, f.c_str(), a.c_str(), goal.x(), goal.y(), priority);
}

void RushSquad::clearGoal()
{
	goal = TilePositions::Invalid;
}

TilePosition RushSquad::getGoal()
{
	return goal;
}

bool RushSquad::hasGoal()
{
	if (goal.x() < 0 || goal.y() < 0)
	{
		return false;
	}
	return true;
}
