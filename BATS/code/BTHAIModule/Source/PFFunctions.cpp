#include "PFFunctions.h"

using namespace BWAPI;
using namespace std;

float PFFunctions::getDistance(const Position& p1, const Position& p2)
{
	return (float)p1.getDistance(p2);
}

float PFFunctions::getDistance(const Position& pos, const Unit* unit)
{
	return (float)unit->getDistance(pos);
}

int PFFunctions::getSize(UnitType type)
{
	if (type.getID() == UnitTypes::Zerg_Zergling)
	{
		//We want Zerglings to spread out
		return 16;
	}
	if (type.isWorker())
	{
		return 6;
	}
	if (type.size() == UnitSizeTypes::Small)
	{
		return 6;
	}
	if (type.size() == UnitSizeTypes::Medium)
	{
		return 12;
	}
	if (type.size() == UnitSizeTypes::Large)
	{
		return 16;
	}
	return 12;
}

float PFFunctions::calcOwnUnitP(float d, const Unit* unit, const Unit* otherOwnUnit)
{
	if (unit == otherOwnUnit)
	{
		//Don't count collision with yourself...
		return 0;
	}

	float p = 0;

	if (BaseAgent::isOfType(otherOwnUnit, UnitTypes::Terran_Vulture_Spider_Mine))
	{
		//Make sure to avoid mines
		if (d <= 125 + getSize(unit->getType()))
		{
			p = -20;
		}
	}

	if (unit->isCloaked() && !otherOwnUnit->isCloaked())
	{
		//Let cloaked units stay away from non-cloaked units to avoid
		//getting killed by splash damage.
		if (d <= 50 + getSize(unit->getType()))
		{
			p = -20;
		}
	}
	if (unit->getType() == UnitTypes::Zerg_Lurker && otherOwnUnit->getType() == UnitTypes::Zerg_Lurker)
	{
		//Let cloaked units stay away from non-cloaked units to avoid
		//getting killed by splash damage.
		if (d <= 64 + getSize(unit->getType()))
		{
			p = -20;
		}
	}

	if (otherOwnUnit->isIrradiated())
	{
		//Other unit under Irradite. Keep distance.
		if (d <= 64)
		{
			p = -20;
		}
	}
	if (otherOwnUnit->isUnderStorm())
	{
		//Other unit under Psionic Storm. Keep distance.
		if (d <= 64)
		{
			p = -20;
		}
	}

    if (d <= getSize(unit->getType()))
{
        p += -20;
    }

    return p;
}

float PFFunctions::calcAvoidWorkerP(float d, BaseAgent* unit, BaseAgent* oUnit)
{
	if (unit == oUnit)
	{
		//Don't count collision with yourself...
		return 0;
	}

	if (oUnit->getUnit()->isGatheringMinerals() || oUnit->getUnit()->isGatheringGas())
	{
		//Workers are transparent when they gather resources
		return 0;
	}

	float p = 0;

    if (d <= 20)
	{
        p = -20;
    }
	else if (d > 20 && d <= 32)
	{
		p = 1;
	}
	
    return p;
}

float PFFunctions::calcMineP(float d, const Unit* unit)
{
	float p = 0;

	if (unit->getType().isFlyer())
	{
		p = 0;
	}
	else if (unit->getType().isFlyingBuilding())
	{
		p = 0;
	}
	else
	{
		if (d <= getSize(unit->getType()))
		{
			p = -40;
		}
	}

	return p;
}

float PFFunctions::calcNavigationP(float d)
{
    float p = 200.0f - 0.1f * d;
    if (p < 0)
    {
        p = 0;
    }
    return p;
}

float PFFunctions::getTrailP(BaseAgent* agent, int cX, int cY)
{
	// Don't use trail for now
	return 0.0f;

	if (agent->getUnit()->isBeingConstructed()) return 0;

	float p = 0.0f;
	Position nPos = Position(cX, cY);

	//Add current position to trail
	//agent->addTrailPosition(agent->getUnit()->getPosition());

	//Get trail
	// vector<Position> trail = agent->getTrail();
	//for (size_t i = 0; i < trail.size(); i++)
	//{
	//	Position tPos = trail.at(i);
	//	//Broodwar->printf("%d: (%d,%d)", i, tPos.x(), tPos.y());

	//	double dist = tPos.getDistance(nPos);
	//	if (dist <= 10.0) p = -5.0;
	//}


	return p;
}

float PFFunctions::getTerrainP(BaseAgent* agent, int cX, int cY)
{
	Position cPos = Position(cX, cY);
	if (!agent->getUnit()->hasPath(cPos))
	{
		return -1000.0;
	}
	
	return 0;
}

float PFFunctions::getGoalP(const Position& cPos, const TilePosition& goal)
{
	if (goal == TilePositions::Invalid) return 0;

	Position gPos = Position(goal);

	float dist = (float)cPos.getDistance(gPos);

	float p = 150.0f - 0.01f * dist;
	if (p < 0)
    {
        p = 0;
    }

	return p;
}

float PFFunctions::getNavigationP(const Position& cPos, const Position& goal)
{
    float dist = getDistance(cPos, goal);
    return calcNavigationP(dist);
}

float PFFunctions::calcAttackingUnitP(float d, const Unit* attacker, const Unit* enemy, bool defensive)
{
	//Check if enemy unit exists and is visible.
	if (!enemy->exists())
	{
		return 0;
	}
	if (!enemy->isVisible())
	{
		return 0;
	}
	if (enemy->isCloaked()) 
	{
		return 0;
	}
	//Check for flying buildings
	if (enemy->getType().isFlyingBuilding() && enemy->isLifted())
	{
		return 0;
	}
	//Check if we can attack the type
	if (!canAttack(attacker, enemy))
	{
		return 0;
	}

	//Calc max wep range
    int myMSD = 0;
	if (enemy->getType().isFlyer())
	{
		myMSD = getAirRange(attacker);
	}
	else
	{
		myMSD = getGroundRange(attacker);
	}

	if (!attacker->getType().canAttack())
	{
		//Unit cannot attack, use sightrange instead
		myMSD = attacker->getType().sightRange();
	}
	if (attacker->getType() == UnitTypes::Terran_Medic)
	{
		myMSD = 6*32;
	}
	if (attacker->getType() == UnitTypes::Protoss_High_Templar)
	{
		myMSD = 6*32;
	}
	if (attacker->getType().getID() == UnitTypes::Zerg_Overlord.getID())
	{
		myMSD = 6*32;
	}

	//Calc attacker wep range
	int enemyMSD = 0;
	if (attacker->getType().isFlyer())
	{
		enemyMSD = getAirRange(enemy);
	}
	else
	{
		enemyMSD = getGroundRange(enemy);
	}
    
    float p = 0;

	//Cloaked unit: Watch out for detectors.
	if (attacker->isCloaked() && enemy->getType().isDetector())
	{
		//defensive = true;
		//enemyMSD = (int)(enemy->getType().sightRange() * 1.2);
	}

	if (!defensive)
	{
		//Offensive mode -> attack
		if (canAttack(attacker, enemy))
		{
			if (d < myMSD - 5)
			{
				float fact = 200.0f / myMSD;
				p = d * fact;
				if (p < 0)
				{
					p = 0;
				}
			}
			else if (d >= myMSD - 5 && d < myMSD)
			{
				p = 200.0f;
			}
			else
			{
				float d1 = d - myMSD;

				p = 180.0f - 0.5f * d1;
				if (p < 0)
				{
					p = 0;
				}
			}
		}
	}
	else
	{
		//Defensive mode -> retreat
		p = -enemyMSD * 1.2f + d;
		if (p > 0) p = 0;
	}

    return p;
}

int PFFunctions::getGroundRange(const Unit* cUnit)
{
	int range = 0;
	if (cUnit->getType().groundWeapon().targetsGround()) 
	{
		int gwR = cUnit->getType().groundWeapon().maxRange();
		if (gwR > range)
		{
			range = gwR;
		}
	}
	if (cUnit->getType().airWeapon().targetsGround()) 
	{
		int gwR = cUnit->getType().airWeapon().maxRange();
		if (gwR > range)
		{
			range = gwR;
		}
	}

	return range;
}

int PFFunctions::getAirRange(const Unit* cUnit)
{
	int range = 0;
	if (cUnit->getType().groundWeapon().targetsAir()) 
	{
		int gwR = cUnit->getType().groundWeapon().maxRange();
		if (gwR > range)
		{
			range = gwR;
		}
	}
	if (cUnit->getType().airWeapon().targetsAir()) 
	{
		int gwR = cUnit->getType().airWeapon().maxRange();
		if (gwR > range)
		{
			range = gwR;
		}
	}

	return range;
}

bool PFFunctions::canAttack(const Unit* ownUnit, const Unit* target)
{
	UnitType oType = ownUnit->getType();
	UnitType tType = target->getType();

	if (tType.isFlyer())
	{
		if (oType.groundWeapon().targetsAir())
		{
			return true;
		}
		else if (oType.airWeapon().targetsAir())
		{
			return true;
		}
	}
	else
	{
		if (oType.groundWeapon().targetsGround())
		{
			return true;
		}
		else if (oType.airWeapon().targetsGround())
		{
			return true;
		}
	}

	return false;
}
