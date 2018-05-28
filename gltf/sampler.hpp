#pragma once
#include "../gl_types.hpp"
#include "json_types.hpp"

namespace rev::gltf {
	struct Sampler {
		int			iLinearMag,
					iLinearMin;
		MipState	mipLevel;
		WrapState	wrapS,
					wrapT;

		Sampler(const JValue& v);
	};
}
