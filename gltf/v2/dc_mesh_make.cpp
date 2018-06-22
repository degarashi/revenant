#include "gltf/v2/dc_mesh.hpp"
#include "gltf/v2/dc_mesh_module.hpp"
#include "gltf/v2/mesh.hpp"
#include "gltf/v2/material.hpp"
#include "gltf/v2/dc_mesh.hpp"

namespace rev::gltf::v2 {
	namespace {
		SkinModule_U MakeSkinModule(const SkinBindSet_SP& bind) {
			return SkinModule_U(new SkinModule(bind));
		}
		SkinModule_U MakeSkinModule(const dc::JointId id) {
			return SkinModule_U(new NoSkinModule(id));
		}
	}

	template <class S_Param>
	HGMesh MakeMesh(const Primitive& p, const WeightV_S& weight, const Name& name, const S_Param& s_param) {
		Module_V module;
		if(p.morph && weight) {
			module.emplace_back(std::make_unique<MorphModule>(*p.morph, weight));
		}
		const PBR_SP& pbr = p.material ? (*p.material)->getPBR() : Material::GetDefaultPBR();
		const bool hasNormalMap = static_cast<bool>(pbr->normal.tex);
		HPrim prim;
		if(hasNormalMap) {
			module.emplace_back(std::make_unique<BumpModule>());
			prim = p.getPrimitiveWithTangent();
		} else {
			prim = p.getPrimitive();
		}
		return HGMesh(new GLTFMesh(
			prim, pbr,
			name, p.getBBox(),
			MakeSkinModule(s_param),
			std::move(module)
		));
	}
	template HGMesh MakeMesh(
		const Primitive& p, const WeightV_S& weight, const Name& name, const dc::JointId& id
	);
	template HGMesh MakeMesh(
		const Primitive& p, const WeightV_S& weight, const Name& name, const SkinBindSet_SP& bind
	);
}
