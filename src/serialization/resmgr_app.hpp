#pragma once
#include "../resmgr_app.hpp"
#include "spine/src/serialization/resmgr_named.hpp"
#include "serialization/apppath.hpp"
#include <cereal/types/base_class.hpp>

namespace rev {
	template <class Ar, class Dat>
	void serialize(Ar& ar, ResMgrApp<Dat>& mgr) {
		using base_t = typename ResMgrApp<Dat>::base_t;
		ar(cereal::base_class<base_t>(&mgr), mgr._cache, mgr._idResType);
	}
}
namespace cereal {
	template <class Ar, class Dat>
	struct specialize<Ar, ::rev::ResMgrApp<Dat>, cereal::specialization::non_member_serialize> {};
}
