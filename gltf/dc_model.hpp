#pragma once
#include "../dc/model_if.hpp"
#include "../dc/qm_cached.hpp"
#include "gltf/dc_common.hpp"

namespace rev::gltf {
	struct Scene;
	class GLTFModel : public dc::IModel {
		private:
			using MeshV = std::vector<HMesh>;
			MeshV		_mesh,
						_skinmesh;
			HTf			_tf;
			mutable dc::QMCached _qm;
		public:
			static HMdl FromScene(const Scene& s);
			GLTFModel(const MeshV& mesh, const MeshV& skinmesh, const HTf& tf);
			void draw(IEffect& e) const override;
			HTf getNode() const override;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
