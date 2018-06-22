#pragma once
#include "../../dc/model_if.hpp"
#include "../../dc/qm_fixed.hpp"
#include "../../dc/bbox.hpp"

namespace rev::gltf::v2 {
	struct IGLTFMesh;
	using HGMesh = std::shared_ptr<IGLTFMesh>;
	class Scene;
	class GLTFModel : public dc::IModel {
		private:
			using MeshV = std::vector<HGMesh>;
			MeshV				_mesh,
								_skinmesh;
			HTf					_tf;
			mutable dc::BSphere_Op	_bsphere;
			mutable dc::QM_Fixed _qm;
		public:
			static HMdl FromScene(const Scene& s);
			GLTFModel(const MeshV& mesh, const MeshV& skinmesh, const HTf& tf);
			void draw(IEffect& e) const override;
			HTf getNode() const override;
			dc::BSphere_Op getBSphere() const override;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
