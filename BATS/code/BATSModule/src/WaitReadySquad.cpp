#include "WaitReadySquad.h"

using namespace bats;
using std::tr1::shared_ptr;

WaitReadySquad::WaitReadySquad(const shared_ptr<Squad>& squad, int timeout) : WaitGoal(timeout) {
	mSquad = squad;
}

WaitReadySquad::~WaitReadySquad() {
	// Does nothing
}

void WaitReadySquad::computeActions() {

}