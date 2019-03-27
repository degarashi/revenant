#pragma once
#include "gltf/v1/dataref.hpp"
#include "../../gl/format.hpp"
#include "../../handle/opengl.hpp"

namespace rev::gltf::v1 {
	struct Texture :
		Resource
	{
		GLInFmt			format,
						internalFormat;
		DRef_Sampler	sampler;
		DRef_Image		source;
		GLTypeFmt		type;

		Texture(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		mutable	HTex	tex_cached;
		const HTex& getGLResource() const;
	};
}
