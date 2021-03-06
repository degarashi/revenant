#pragma once
#include "../../effect/vertex.hpp"
#include "../json_types.hpp"
#include "spine/src/optional.hpp"

namespace rev::gltf::v1 {
	spi::Optional<VSemantic> VSemanticFromString(const char* s);
	struct V_Semantic {
		VSemantic operator()(const char* s) const;
	};
}
