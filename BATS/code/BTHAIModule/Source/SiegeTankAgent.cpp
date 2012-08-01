#include "SiegeTankAgent.h"
#include "PFManager.h"
#include "TargetingAgent.h"

using namespace BWAPI;
using namespace std;

SiegeTankAgent::SiegeTankAgent(Unit* mUnit) : UnitAgent(mUnit)
{
	agentType = "SiegeTankAgent";
	resetToDefaultBehavior();
}

void SiegeTankAgent::computeActions()
{
	// Only check the various siege modes if we actually have siege mode.
	if (Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode))
	{
		switch (mSiegeMode) {
			case SiegeMode_None:
				// Does nothing
				break;

			case SiegeMode_Auto: {
				int eCnt = enemyGroundUnitsWithinRange(getGroundRange(UnitTypes::Terran_Siege_Tank_Siege_Mode));
				if (eCnt > 0 && !unit->isSieged())
				{
					unit->siege();
					return;
				}
				if (eCnt == 0 && unit->isSieged())
				{
					unit->unsiege();
					return;
				}
				break;
			}

			case SiegeMode_Siege:
				if (!unit->isSieged()) {
					unit->siege();
					return;
				}
				break;

			case SiegeMode_Tank:
				if (unit->isSieged()) {
					unit->unsiege();
					return;
				}
				break;
		}
		
	}

	findAndTryAttack();

	// Only try to move when in normal mode (cannot move while sieged).
	if (!unit->isSieged())
	{
		computeMoveAction();
	}
}

void SiegeTankAgent::setAutoSiegeMode(bool on) {
	if (on) {
		mSiegeMode = SiegeMode_Auto;
	} else {
		mSiegeMode = SiegeMode_None;
	}
}

void SiegeTankAgent::forceSiegeMode() {
	mSiegeMode = SiegeMode_Siege;
}

void SiegeTankAgent::forceTankMode() {
	mSiegeMode = SiegeMode_Tank;
}

void SiegeTankAgent::resetToDefaultBehavior() {
	mSiegeMode = SiegeMode_Auto;
}