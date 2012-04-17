#include "Helper.h"
#include <ostream>

std::ostream& bats::operator<<(std::ostream& out, const BWAPI::TilePosition& position) {
	out << "(" << position.x() << ", " << position.y() << ")";
	return out;
}

/**
 * \copydoc operator<<(std::ostream&,BWPAI::Tileposition&)
 */
std::ostream& bats::operator<<(std::ostream& out, const BWAPI::Position& position) {
	out << "(" << position.x() << ", " << position.y() << ")";
	return out;
}