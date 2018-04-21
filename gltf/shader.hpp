#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "../gl_types.hpp"
#include "../handle/opengl.hpp"
#include <GL/gl.h>

namespace rev::gltf {
	struct Shader :
		Resource,
		IResolvable
	{
		TagRW		src;
		ShType		type;

		Shader(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;

		HSh makeShader() const;
	};
}
