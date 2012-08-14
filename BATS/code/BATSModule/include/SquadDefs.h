#pragma once

#include "Utilities/KeyType.h"
#include <memory>

namespace bats {
	struct _SquadType{};
	typedef utilities::KeyType<bats::_SquadType> SquadId;

	struct _PlayerSquadType{};
	typedef utilities::KeyType<bats::_PlayerSquadType> PlayerSquadId;

	class PlayerSquad;
	typedef std::tr1::shared_ptr<PlayerSquad> PlayerSquadPtr;
	typedef std::tr1::shared_ptr<const PlayerSquad> PlayerSquadCstPtr;
	typedef const std::tr1::shared_ptr<PlayerSquad>& PlayeSquadRef;

	class AlliedSquad;
	typedef std::tr1::shared_ptr<AlliedSquad> AlliedSquadPtr;
	typedef std::tr1::shared_ptr<const AlliedSquad> AlliedSquadCstPtr;
	typedef const std::tr1::shared_ptr<AlliedSquad>& AlliedSquadRef;

	class EnemySquad;
	typedef std::tr1::shared_ptr<EnemySquad> EnemySquadPtr;
	typedef std::tr1::shared_ptr<const EnemySquad> EnemySquadCstPtr;
	typedef const std::tr1::shared_ptr<EnemySquad>& EnemySquadRef;

	class Squad;
	typedef std::tr1::shared_ptr<Squad> SquadPtr;
	typedef std::tr1::shared_ptr<const Squad> SquadCstPtr;
	typedef const std::tr1::shared_ptr<Squad>& SquadRef;

	class AttackSquad;
	typedef std::tr1::shared_ptr<AttackSquad> AttackSquadPtr;
	typedef std::tr1::shared_ptr<const AttackSquad> AttackSquadCstPtr;
	typedef const std::tr1::shared_ptr<AttackSquad>& AttackSquadRef;

	class PatrolSquad;
	typedef std::tr1::shared_ptr<PatrolSquad> PatrolSquadPtr;
	typedef std::tr1::shared_ptr<const PatrolSquad> PatrolSquadCstPtr;
	typedef const std::tr1::shared_ptr<PatrolSquad>& PatrolSquadRef;

	class HoldSquad;
	typedef std::tr1::shared_ptr<HoldSquad> HoldSquadPtr;
	typedef std::tr1::shared_ptr<const HoldSquad> HoldSquadCstPtr;
	typedef const std::tr1::shared_ptr<HoldSquad>& HoldSquadRef;
}