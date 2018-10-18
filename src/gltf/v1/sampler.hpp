#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/sampler.hpp"

namespace rev::gltf::v1 {
	struct Sampler :
		gltf::Sampler,
		Resource
	{
		Sampler(const JValue& v);
		Type getType() const noexcept override;
	};
}
