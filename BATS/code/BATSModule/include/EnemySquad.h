#pragma once

#include "PlayerSquad.h"

// Namespace for the project
namespace bats {

/**
 * Virtual squad for grouping enemy units together.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class EnemySquad : public PlayerSquad {
public:
	/**
	 * Default constructor
	 */
	EnemySquad();

	/**
	 * Destructor
	 */
	virtual ~EnemySquad();

protected:
	virtual void updateDerived();
	virtual bool isDebugOff() const;

private:

};
}