#pragma once

#include "Utilities/KeyType.h"
#include <memory>

namespace bats {
	struct _SquadType{};
	typedef utilities::KeyType<bats::_SquadType> SquadId;

	struct _AlliedSquadType{};
	typedef utilities::KeyType<bats::_AlliedSquadType> AlliedSquadId;

	class AlliedSquad;
	typedef std::tr1::shared_ptr<AlliedSquad> AlliedSquadPtr;
	typedef std::tr1::shared_ptr<const AlliedSquad> AlliedSquadCstPtr;
}