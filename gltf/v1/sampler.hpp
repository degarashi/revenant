#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/json_types.hpp"
#include "../../gl_types.hpp"

namespace rev::gltf::v1 {
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
