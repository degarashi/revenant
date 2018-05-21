#pragma once
#include "../dc/model.hpp"
#include "gltf/dc_common.hpp"

namespace rev::gltf {
	using Name = std::string;
	// Mesh, SkinMesh共用
	class GLTFModel : public dc::IModel {
		public:
			GLTFModel(const MeshV& mesh, const HTf& tf);
			void draw(IEffect& e) const override;
	};
}
