#pragma once
#include "tech_if.hpp"

namespace rev {
	namespace parse {
		class BlockSet;
	}
	using Tech_SPV = std::vector<HTech>;
	struct TechPair {
		Name		name;
		Tech_SPV	pass;
	};
	using TechPairV = std::vector<TechPair>;
	TechPairV MakeGLXMaterial(const parse::BlockSet& bs);
}
