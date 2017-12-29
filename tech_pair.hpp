#pragma once
#include "tech_if.hpp"

namespace rev {
	namespace parse {
		class BlockSet;
		using BlockSet_SP = std::shared_ptr<BlockSet>;
	}
	using Tech_SPV = std::vector<Tech_SP>;
	struct TechPair {
		Name		name;
		Tech_SPV	pass;
	};
	using TechPairV = std::vector<TechPair>;
	TechPairV MakeGLXMaterial(const parse::BlockSet_SP& bs);
}
