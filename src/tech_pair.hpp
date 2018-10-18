#pragma once
#include "handle/opengl.hpp"

namespace rev {
	using Name = std::string;
	using Tech_SPV = std::vector<HTech>;
	struct TechPair {
		Name		name;
		Tech_SPV	pass;
	};
	namespace parse {
		class BlockSet;
	}
	using TechPairV = std::vector<TechPair>;
	TechPairV MakeTechPair(const parse::BlockSet& bs);
}
