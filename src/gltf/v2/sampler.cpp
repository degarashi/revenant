#include "gltf/v2/sampler.hpp"

namespace rev::gltf::v2 {
	Sampler::Sampler(const JValue& v):
		gltf::Sampler(v),
		Resource(v)
	{}
	Sampler::Sampler():
		Resource(nullptr)
	{
		iLinearMag = 1;
		iLinearMin = 1;
		mipLevel = MipState::NoMipmap;
		wrapS = wrapT = WrapState::Repeat;
	}
	Resource::Type Sampler::getType() const noexcept {
		return Type::Sampler;
	}
	Sampler Sampler::DefaultSampler() {
		return {};
	}
}
