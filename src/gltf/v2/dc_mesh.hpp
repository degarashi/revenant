#pragma once
#include "gltf/v2/common.hpp"
#include "gltf/v2/dc_mesh_if.hpp"
#include "../../singleton_data.hpp"

namespace rev {
	struct PBRMetallicRoughness;
	using PBR_SP = std::shared_ptr<PBRMetallicRoughness>;
	namespace dc {
		struct IQueryMatrix;
	}
}
namespace rev::gltf::v2 {
	struct ISkinModule;
	using SkinModule_U = std::unique_ptr<ISkinModule>;
	struct IModule;
	using Module_U = std::unique_ptr<IModule>;
	using Module_V = std::vector<Module_U>;
	struct Primitive;

	class GLTFMesh : public IGLTFMesh {
		private:
			using Bool_OP = spi::Optional<bool>;
		protected:
			HPrim				_primitive;
			PBR_SP				_pbr;
			Name				_name;
			mutable dc::BBox_Op	_bbox;
			mutable Bool_OP		_flip;

			SkinModule_U		_skin;
			Module_V			_module;
			HTech				_tech;

			// ------- shared data -------
			using Environment = SingletonData<GLTexture, GLTFMesh>;
			Environment			_env;
			using TP = SingletonData<TechPass, GLTFMesh>;
			TP					_tp;

			template <class S_Param>
			friend HGMesh MakeMesh(const Primitive& p, const WeightV_S& weight, const Name& name, const S_Param& s_param);
			GLTFMesh(const HPrim& p, const PBR_SP& pbr,
					const Name& name, dc::BBox_Op bbox,
					SkinModule_U skin, Module_V module);

		public:
			static HTex MakeData(Environment*);
			static HTP MakeData(TP*);

			HTech getTech() const override;
			void draw(IEffect& e, const dc::IQueryMatrix& qm, dc::BBox* bb) const override;
			DEF_DEBUGGUI_SUMMARYSTR
			DEF_DEBUGGUI_PROP
	};
}
