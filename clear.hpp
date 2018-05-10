#pragma once
#include "spine/optional.hpp"
#include "frea/vector.hpp"

namespace rev {
	struct ClearParam {
		spi::Optional<frea::Vec4>	color;
		spi::Optional<float>		depth;
		spi::Optional<uint32_t>		stencil;
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::ClearParam)
