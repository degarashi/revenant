#pragma once
#include "../json_types.hpp"
#include "spine/optional.hpp"

namespace rev::gltf::v2 {
	using String = std::string;
	using String_OP = spi::Optional<std::string>;
	struct Asset {
		String_OP		copyright,
						generator,
						minVersion;
		String			version;

		Asset(const JValue& v);
	};
}
