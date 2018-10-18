#pragma once
#include "../../dc/model_if.hpp"
#include "../../dc/qm_fixed.hpp"
#include "gltf/v1/dc_common.hpp"

namespace rev::gltf::v1 {
	struct Scene;
	class GLTFModel : public dc::IModel {
		private:
			using MeshV = std::vector<HMesh_U>;
			MeshV		_mesh,
						_skinmesh;
			HTf			_tf;
			mutable dc::QM_Fixed _qm;
		public:
			static HMdl FromScene(const Scene& s);
			GLTFModel(const MeshV& mesh, const MeshV& skinmesh, const HTf& tf);
			void draw(IEffect& e) const override;
			HTf getNode() const override;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
