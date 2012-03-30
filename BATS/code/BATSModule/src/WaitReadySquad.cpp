#include "WaitReadySquad.h"

using namespace bats;
using std::tr1::shared_ptr;

WaitReadySquad::WaitReadySquad(const shared_ptr<AttackSquad>& squad, double timeout) : WaitGoal(timeout) {
	mSquad = squad;
}

WaitReadySquad::~WaitReadySquad() {
	// Does nothing
}

void WaitReadySquad::update() {
	WaitGoal::update();

	if (mWaitState == WaitState_Waiting) {
		if (mSquad->isReadyToAttack()) {
			mWaitState = WaitState_Success;
		} else if (mSquad->isEmpty()) {
			mWaitState = WaitState_Failed;
		}
	}
}