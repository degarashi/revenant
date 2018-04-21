/*
#include "gltf/uniform_value.hpp"
#include "gltf/idtag.hpp"
#include "gltf/texture.hpp"

namespace rev {
	namespace gltf {
		UniformValue_UP LoadUniformValue(const JValue& v) {
			if(auto p = UniformArray<loader::Bool, 4>::Load(v))
				return p;
			if(auto p = UniformArray<loader::Number, 4>::Load(v))
				return p;
			if(auto p = UniformMat<2>::Load(v))
				return p;
			if(auto p = UniformMat<3>::Load(v))
				return p;
			if(auto p = UniformMat<4>::Load(v))
				return p;
			if(auto p = UniformTag<TagTexture, 4>::Load(v))
				return p;
			throw InvalidProperty("unknown uniform value");
		}
	}
}
*/
