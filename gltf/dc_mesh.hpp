#pragma once
#include "../dc/mesh_if.hpp"
#include "gltf/dc_common.hpp"

namespace rev::gltf {
	struct IQueryMatrix;
	using Name = std::string;
	// Mesh, SkinMesh共用
	class GLTFMesh : public dc::IMesh {
		public:
			HPrim		_primitive;
			HTech		_tech;
			Name		_userName;
			// 描画毎にSemanticに応じて値を算出、設定する物
			RTUParams_SP		_rtParams;
			dc::JointId			_jointId;
			SkinBindSet_SP		_bind;

			// Uniform-Idのキャッシュ
			using UIdV = std::vector<int>;
			UIdV				_uId;
		public:
			GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, dc::JointId id);
			GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const SkinBindSet_SP& bind);
			void draw(IEffect& e, const dc::IQueryMatrix& qm) const override;
			HTech getTech() const override;
			DEF_DEBUGGUI_SUMMARYSTR
			DEF_DEBUGGUI_PROP
	};
}
