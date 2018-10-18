#include "gltf/v1/sampler.hpp"

namespace rev::gltf::v1 {
	Sampler::Sampler(const JValue& v):
		gltf::Sampler(v),
		Resource(v)
	{}
	Resource::Type Sampler::getType() const noexcept {
		return Type::Sampler;
	}
}
