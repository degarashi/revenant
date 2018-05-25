#pragma once
#include "resource.hpp"
#include "dataref.hpp"
#include "../gl_format.hpp"
#include "../tech.hpp"
#include "gltf/dc_common.hpp"
#include "gltf/uniform_value.hpp"

namespace rev::gltf {
	struct Material :
		Resource
	{
		DRef_Technique		technique;

		// Uniform変数の上書き分
		UniformValueNVP		uniformOvr;

		struct GLTfTech : Tech {
			GLTfTech(const Material& mtl);
		};
		mutable HTech			tech_cached;

		Material(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		const UniformValue* findValue(const Name& name) const;
		const HTech& getTech() const;
		RTUParams_SP getRT() const;
	};
}
