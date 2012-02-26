#pragma once

/**
 * A macro to check if something (e.g. unit) belongs to us
 * @param what does this belong to us?
 * @return true if the what belongs to us, else false
 * @pre what needs to return a BWAPI::Player by the function getPlayer.
 * @pre what needs to be a pointer.
 * @pre a global variable 'Broodwar' should be set.
 */
#define OUR(what) (what->getPlayer()->getID() == Broodwar->self()->getID())