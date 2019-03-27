#pragma once
#include "../gl/types.hpp"
#include "json_types.hpp"

namespace rev::gltf {
	struct Sampler {
		int			iLinearMag,
					iLinearMin;
		MipState	mipLevel;
		WrapState	wrapS,
					wrapT;

		Sampler() = default;
		Sampler(const JValue& v);
	};
}
