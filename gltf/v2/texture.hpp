#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/v2/dataref.hpp"
#include "../../handle/opengl.hpp"

namespace rev::gltf::v2 {
	class Texture :
		public Resource
	{
		private:
			Sampler _getSampler() const;
		public:
			DRef_Sampler_OP		sampler;
			DRef_Image_OP		source;

			Texture(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;

			mutable HTex		tex_cached;
			const HTex& getGLResource() const;
	};
}
