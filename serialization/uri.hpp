#pragma once
#include "../uri.hpp"
#include "serialization/path.hpp"

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, URI& uri) {
		ar(cereal::base_class<PathBlock>(&uri), uri._type);
	}
}
namespace cereal {
	template <class Ar>
	struct specialize<Ar, ::rev::URI, cereal::specialization::non_member_serialize> {};
}
