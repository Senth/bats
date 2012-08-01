#ifndef __SIEGETANKAGENT_H__
#define __SIEGETANKAGENT_H__

#include <BWAPI.h>
#include "UnitAgent.h"

/** The SiegeTankAgent handles Terran Siege Tank units.
 *
 * @section Special Abilities
 * \li setAutoSiege() (enabled by default): Enters siege mode if enemy units are close by and siege
 * mode has been researched.
 * \li forceSiegeMode(): Forces the tank into siege mode until either auto-siege mode is enabled or
 * forceTankMode() is called. Disables auto-siege mode.
 * \li forceTankMode(): Forces the tank to unsiege and never siege until either auto-siege mode is
 * enabled or forceSiegeMode() is called. Disables auto-siege mode.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 * Added ability to turn on/off auto-siege mode and forcing siege/unsiege.
 */
class SiegeTankAgent : public UnitAgent {
	
public:
	
	SiegeTankAgent(BWAPI::Unit* mUnit);

	virtual void computeActions();

	/**
	 * Turns on/off auto-siege mode. When it is turned on the tank will automatically siege when
	 * enemies are close by (if siege mode has been researched). If no enemies are present it will
	 * automatically unsiege. If auto-siege mode is turned off the tank will remain in its current
	 * mode. This function turns of forceSiegeMode() or forceTankMode() if they were active.
	 * @note If turned off and the player sieges/unsieges the tank through the interface it will
	 * remain in that new state.
	 * @param on set to true if the siege mode shall be turned on, false if turned off.
	 * @see forceSiegeMode()
	 * @see forceTankMode()
	 */
	void setAutoSiegeMode(bool on);

	/**
	 * Forces the tank to always be in siege mode. If it isn't it will automatically siege.
	 * @note Only works if siege mode is researched
	 * @see setAutoSiegeMode()
	 * @see forceTankMode()
	 */
	void forceSiegeMode();

	/**
	 * Forces the tank to always be in normal mode. If the tank is in siege mode it will
	 * automatically unsiege (checked every time).
	 * @note Tank mode is the normal mode when the tank can move.
	 * @see setAutoSiegeMode()
	 * @see forceSiegeMode()
	 */
	void forceTankMode();

	/**
	 * Resets the tank its default behavior:
	 * \li Auto-siege mode is turned on
	 */
	virtual void resetToDefaultBehavior();

private:
	/**
	 * Different siege modes for the tank
	 */
	enum SiegeModes {
		SiegeMode_None, /**< The tank continues to be in the current state */
		SiegeMode_Auto, /**< Sieges when enemies are close by and unsieges when they are gone */
		SiegeMode_Siege, /**< Forces siege mode all the time */
		SiegeMode_Tank /**< Forces tank mode (unsieged) all the time */
	};

	SiegeModes mSiegeMode;
};

#endif
