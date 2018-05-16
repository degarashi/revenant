#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "../handle/opengl.hpp"

namespace rev::gltf {
	struct Program :
		Resource,
		IResolvable
	{
		// とりあえず無視
		GLSLNameV	attribute;
		TagShader	vshader,
					fshader;

		Program(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;

		mutable HProg cache;
		const HProg& makeProgram() const;
	};
}
