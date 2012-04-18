#include "KiteSquad.h"
#include "UnitAgent.h"
#include "AgentManager.h"
#include "ExplorationManager.h"
#include "Commander.h"

using namespace BWAPI;
using namespace BWTA;
using namespace std;

KiteSquad::KiteSquad(int mId, string mName, int mPriority)
{
	this->id = mId;
	this->type = KITE;
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

bool KiteSquad::isActive()
{
	return active;
}

void KiteSquad::defend(TilePosition mGoal)
{
	if (!active)
	{
		setGoal(mGoal);
	}
}

void KiteSquad::attack(TilePosition mGoal)
{

}

void KiteSquad::assist(TilePosition mGoal)
{
	if (!active)
	{
		setGoal(mGoal);
	}
}

void KiteSquad::computeActions()
{
	if (!active)
	{

		if (isFull())
		{
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

	//All units dead, go back to inactive
	if ((int)agents.size() == 0)
	{
		active = false;
		return;
	}

	if (active)
	{
		if (activePriority != priority)
		{
			priority = activePriority;
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
			this->goal = ePos;
			setMemberGoals(goal);
		}
	}
}

TilePosition KiteSquad::getNextStartLocation()
{
	for(set<BaseLocation*>::const_iterator i=getStartLocations().begin();i!=getStartLocations().end();i++)
	{
		TilePosition basePos = (*i)->getTilePosition();
		if (!isVisible(basePos))
		{
			return basePos;
		}
		else
		{
			if ((int)agents.size() > 0)
			{
				UnitAgent* uagent = (UnitAgent*)agents.at(0);
				int eCnt = uagent->enemyUnitsWithinRange(10 * 32);
				if (eCnt > 0)
				{
					return TilePositions::Invalid;
				}
			}

			hasVisited.push_back(basePos);
		}
	}
	return TilePositions::Invalid;
}

bool KiteSquad::isVisible(TilePosition pos)
{
	if (!ExplorationManager::canReach(Broodwar->self()->getStartLocation(), pos))
	{
		return true;
	}

	if (Broodwar->isVisible(pos))
	{
		return true;
	}

	if (getCenter().getDistance(pos) <= 3)
	{
		return true;
	}

	for (int i = 0; i < (int)hasVisited.size(); i++)
	{
		TilePosition vPos = hasVisited.at(i);
		if (vPos.x() == pos.x() && vPos.y() == pos.y())
		{
			return true;
		}
	}

	return false;
}

void KiteSquad::printInfo()
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

	Broodwar->printf("[KiteSquad %d] (%s, %s) Goal: (%d,%d) prio: %d", id, f.c_str(), a.c_str(), goal.x(), goal.y(), priority);
}

void KiteSquad::clearGoal()
{
	
}

TilePosition KiteSquad::getGoal()
{
	return goal;
}

bool KiteSquad::hasGoal()
{
	if (goal.x() < 0 || goal.y() < 0)
	{
		return false;
	}
	return true;
}
