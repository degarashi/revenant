#pragma once
#include "vertex.hpp"
#include "lubee/src/assoc.hpp"

namespace rev {
	struct VSemAttrMap : lubee::AssocVec<VSem_AttrId> {};
}
namespace std {
	template <>
	struct hash<rev::VSemAttrMap> {
		std::size_t operator()(const rev::VSemAttrMap& m) const noexcept {
			return std::hash<lubee::AssocVec<rev::VSem_AttrId>>()(m);
		}
	};
}
