#pragma once
#include "dataref.hpp"
#include "../gl_format.hpp"
#include "../handle/opengl.hpp"

namespace rev::gltf {
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

		HTex getGLResource() const;
	};
}
