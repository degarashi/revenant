#pragma once
#include "resource.hpp"
#include "gltf/v1/dataref.hpp"
#include "../../gl_format.hpp"
#include "../../tech.hpp"
#include "gltf/v1/dc_common.hpp"
#include "gltf/v1/uniform_value.hpp"

namespace rev::gltf::v1 {
	struct Material :
		Resource
	{
		DRef_Technique		technique;

		// Uniform変数の上書き分
		UniformValueNVP		uniformOvr;

		class GLTfTech : public Tech {
			private:
				using MakeUniformF = std::function<void (UniformEnt&)>;
				MakeUniformF _makeUniformF;

				void _makeUniform(UniformEnt& u) const override;
			public:
				GLTfTech(const Material& mtl);
		};
		mutable HTech			tech_cached;

		Material(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		const HTech& getTech() const;
		RTUParams_SP getRT() const;
	};
}
