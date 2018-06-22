#pragma once
#include "../../vertex.hpp"
#include "../json_types.hpp"
#include "spine/optional.hpp"

namespace rev::gltf::v2 {
	spi::Optional<VSemantic> VSemanticFromString(const char* s);
	struct V_Semantic {
		VSemantic operator()(const char* s) const;
	};
}
