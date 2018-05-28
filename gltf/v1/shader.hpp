#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/rwref.hpp"
#include "../../gl_types.hpp"
#include "../../handle/opengl.hpp"
#include "../../gl_header.hpp"

namespace rev::gltf::v1 {
	struct Shader :
		Resource
	{
		RWRef		src;
		ShType		type;

		Shader(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		HSh makeShader() const;
	};
}
