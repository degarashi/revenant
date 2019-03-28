#pragma once
#include "spine/src/optional.hpp"
#include "frea/src/vector.hpp"

namespace rev {
	struct ClearParam {
		spi::Optional<frea::Vec4>	color;
		spi::Optional<float>		depth;
		spi::Optional<uint32_t>		stencil;
	};
}
#include "../lua/import.hpp"
DEF_LUAIMPORT(rev::ClearParam)
