#include "WaitReadySquad.h"

using namespace bats;
using std::tr1::shared_ptr;

WaitReadySquad::WaitReadySquad(const shared_ptr<AttackSquad>& squad, int timeout) : WaitGoal(timeout) {
	mSquad = squad;
}

WaitReadySquad::~WaitReadySquad() {
	// Does nothing
}

void WaitReadySquad::computeActions() {
	WaitGoal::computeActions();

	if (mWaitState == WaitState_Waiting) {
		if (mSquad->isReadyToAttack()) {
			mWaitState = WaitState_Success;
		}
	}
}