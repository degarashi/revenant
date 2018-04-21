#pragma once
#include "resource.hpp"
#include "../gl_types.hpp"

namespace rev::gltf {
	struct Sampler : Resource {
		int			iLinearMag,
					iLinearMin;
		MipState	mipLevel;
		WrapState	wrapS,
					wrapT;

		Sampler(const JValue& v);
		Type getType() const noexcept override;
	};
}
