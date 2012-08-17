#include "AlliedClassifier.h"
#include <cstdlib> // For NULL
#include <BWAPI/Game.h>

using namespace bats;
using namespace BWAPI;
using namespace std;

AlliedClassifier* AlliedClassifier::msInstance = NULL;

AlliedClassifier::AlliedClassifier() {
	// Does nothing
}

AlliedClassifier::~AlliedClassifier() {
	msInstance = NULL;
}

AlliedClassifier* AlliedClassifier::getInstance() {
	if (NULL == msInstance) {
		msInstance = new AlliedClassifier();
	}
	return msInstance;
}

bool AlliedClassifier::isExpanding() const {
	const set<Player*>& allies = Broodwar->allies();
	set<Player*>::const_iterator allyIt;
	for (allyIt = allies.begin(); allyIt != allies.end(); ++allyIt) {
		const set<Unit*>& units = Broodwar->getAllUnits();
		set<Unit*>::const_iterator unitIt;
		for (unitIt = units.begin(); unitIt != units.end(); ++unitIt) {
			if ((*unitIt)->getType().isResourceDepot() && !(*unitIt)->isCompleted()) {
				return true;
			}
		}
	}

	return false;
}