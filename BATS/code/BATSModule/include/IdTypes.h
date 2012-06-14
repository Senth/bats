#pragma once

#include "Utilities/KeyType.h"

namespace bats {
	struct _SquadType{};
	/**
	 * Shortcut for Squad ids.
	 */
	typedef utilities::KeyType<bats::_SquadType> SquadId;
	struct _AlliedSquadType{};
	typedef utilities::KeyType<bats::_AlliedSquadType> AlliedSquadId;
}