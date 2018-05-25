#pragma once
#include "gltf/resource.hpp"
#include "gltf/dataref.hpp"
#include "../handle/opengl.hpp"

namespace rev::gltf {
	struct Program :
		Resource
	{
		// とりあえず無視
		GLSLNameV	attribute;
		DRef_Shader	vshader,
					fshader;

		Program(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		mutable HProg cache;
		const HProg& makeProgram() const;
	};
}
