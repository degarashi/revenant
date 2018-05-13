#pragma once
#include "../dc/mesh.hpp"
#include "gltf/dc_common.hpp"

namespace rev::gltf {
	using Name = std::string;
	// Mesh, SkinMesh共用
	class GLTFMesh : public dc::IMesh {
		private:
			// 描画毎にSemanticに応じて値を算出、設定する物
			RTUParams_SP		_rtParams;
			dc::JointId			_jointId;
			dc::SkinBindV_SP	_bind;
			frea::Mat4			_bsm;
		public:
			GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, dc::JointId id);
			GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::SkinBindV_SP& bind, const frea::Mat4& bsm);
			// setterした後にbase::drawを呼ぶ
			void draw(IEffect& e, const dc::NodeParam& np) const override;
			DEF_DEBUGGUI_SUMMARYSTR
			DEF_DEBUGGUI_PROP
	};
}
