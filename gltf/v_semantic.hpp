#pragma once
#include "../vertex.hpp"
#include "spine/optional.hpp"

namespace rev::gltf {
	struct V_Semantic {
		std::string		sem;

		static spi::Optional<VSemantic> FromString(const char* s);
		bool operator == (const std::string& s) const noexcept;
		bool operator == (const char* s) const noexcept;
	};
}
