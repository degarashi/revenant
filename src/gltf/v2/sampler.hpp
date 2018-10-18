#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/sampler.hpp"

namespace rev::gltf::v2 {
	class Sampler :
		public gltf::Sampler,
		public Resource
	{
		private:
			Sampler();
		public:
			static Sampler DefaultSampler();
			Sampler(const JValue& v);
			Type getType() const noexcept override;
	};
}
