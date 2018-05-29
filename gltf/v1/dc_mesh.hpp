#pragma once
#include "../../dc/mesh_if.hpp"
#include "gltf/v1/dc_common.hpp"
#include "spine/optional.hpp"

namespace rev::gltf::v1 {
	struct IQueryMatrix_USem;
	using Name = std::string;
	// Mesh, SkinMesh共用
	class GLTFMesh : public dc::IMesh {
		public:
			using Bool_OP = spi::Optional<bool>;

			HPrim		_primitive;
			HTech		_tech;
			Name		_userName;
			// 描画毎にSemanticに応じて値を算出、設定する物
			RTUParams_SP		_rtParams;
			dc::JointId			_jointId;
			SkinBindSet_SP		_bind;
			mutable Bool_OP		_flip;

			// Uniform-Idのキャッシュ
			using UIdV = std::vector<int>;
			UIdV				_uId;
		public:
			GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, dc::JointId id);
			GLTFMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const SkinBindSet_SP& bind);
			bool draw(IEffect& e, const IQueryMatrix_USem& qm) const;
			HTech getTech() const override;
			DEF_DEBUGGUI_SUMMARYSTR
			DEF_DEBUGGUI_PROP
	};
}
