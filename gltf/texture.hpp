#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "../gl_format.hpp"
#include "../handle/opengl.hpp"

namespace rev::gltf {
	struct Texture :
		Resource,
		IResolvable
	{
		GLInFmt		format,
					internalFormat;
		TagSampler	sampler;
		TagImage	source;
		GLTypeFmt	type;

		Texture(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;

		HTex getGLResource() const;
	};
}
