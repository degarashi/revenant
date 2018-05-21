#pragma once
#include "../dc/model_if.hpp"
#include "gltf/dc_common.hpp"

namespace rev::gltf {
	// Mesh, SkinMesh共用
	class GLTFModel : public dc::IModel {
		private:
			using MeshV = std::vector<HMesh>;
			MeshV		_mesh,
						_skinmesh;
			HTf			_tf;
		public:
			GLTFModel(const MeshV& mesh, const MeshV& skinmesh, const HTf& tf);
			void draw(IEffect& e) const override;
			HTf getNode() const override;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
