#include "EnemySquad.h"
#include "Config.h"

using namespace bats;

EnemySquad::EnemySquad() {
	// Does nothing
}

EnemySquad::~EnemySquad() {
	// Does nothing
}

void EnemySquad::updateDerived() {
	// Does nothing
}

bool EnemySquad::isDebugOff() const {
	return config::debug::modules::ENEMY_SQUAD == false;
}