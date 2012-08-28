#include "VultureAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

VultureAgent::VultureAgent(BWAPI::Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "VultureAgent";
	mineDropFrame = 0;
}

void VultureAgent::computeActions()
{
	/// @todo kite
	//Squad* sq = Commander::getInstance()->getSquad(squadID);
	//if (sq != NULL)
	//{
	//	if (sq->isKite())
	//	{
	//		computeKitingActions();
	//		return;
	//	}
	//}

	int eCnt = enemyGroundAttackingUnitsWithinRange(320);
	if (eCnt > 0)
	{
		int framesSinceDrop = Broodwar->getFrameCount() - mineDropFrame;
		if (unit->getSpiderMineCount() > 0 && framesSinceDrop >= 100)
		{
			//Broodwar->printf("[%d] dropped spider mine", unitID);
			unit->useTech(TechTypes::Spider_Mines, unit->getPosition());
			mineDropFrame = Broodwar->getFrameCount();
			return;
		}
	}

	/// @todo drop mines near choke points

	bool attackingEnemy = findAndTryAttack();
	if (!attackingEnemy) {
		computeMoveAction();
	}
}
