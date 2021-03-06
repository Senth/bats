#pragma once

#include "WaitGoal.h"
#include <memory.h>
#include "AttackSquad.h"

// Namespace for the project
namespace bats {

/**
 * Waits for a squad until it gets into position and is ready to attack.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class WaitReadySquad : public WaitGoal {
public:
	/**
	 * Waits for the specified squad to get into position and is ready to attack.
	 * @param squad the squad to wait for.
	 * @param timeout how many game seconds (on fastest speed) we max wait until the goal will
	 * timeout.
	 */
	WaitReadySquad(const std::tr1::shared_ptr<AttackSquad>& squad, double timeout);

	/**
	 * Destructor
	 */
	virtual ~WaitReadySquad();

	/**
	 * Calculates whether the squad is ready or not, and if it has timed out.
	 * @see getWaitState() for the current state.
	 */
	void update();
private:
	std::tr1::shared_ptr<AttackSquad> mSquad;
};
}