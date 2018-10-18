#pragma once
#include "gltf/v2/skinbind.hpp"
#include "gltf/v2/common.hpp"
#include "../../dc/mesh_if.hpp"
#include "../../dc/bbox.hpp"

namespace rev::gltf::v2 {
	using SkinBindSet_SP = std::shared_ptr<SkinBindSet>;
	using WeightV = std::vector<float>;
	using WeightV_S = std::shared_ptr<WeightV>;

	struct IGLTFMesh;
	using HGMesh = std::shared_ptr<IGLTFMesh>;
	struct IGLTFMesh : dc::IMesh {
		virtual void draw(IEffect& e, const dc::IQueryMatrix& qm, dc::BBox* bb) const = 0;
	};
	struct Primitive;
	template <class S_Param>
	HGMesh MakeMesh(const Primitive& p, const WeightV_S& weight, const Name& name, const S_Param& s_param);
}
