#include "param.hpp"
#include "gltf2_scene.hpp"
#include "resmgr_app.hpp"
#include "shared.hpp"
#include "../../drawutil/gle_nest.hpp"
#include "../../effect/u_common.hpp"
#include "../../effect/u_matrix2d.hpp"
#include "../../effect/u_matrix3d.hpp"
#include "../../effect/u_pbr_mr.hpp"
#include "../../gl/resource.hpp"
#include "../../effect/sys_uniform.hpp"
#include "../../u_morph.hpp"

namespace rev::test {
	HFx Param::makeEffect() const {
		return mgr_gl.template makeResource<
			drawutil::GLE_Nest<
				SystemUniform,
				U_Matrix2D,
				U_Matrix3D,
				U_Common,
				U_PBRMetallicRoughness,
				U_Morph,
				GLEffect
			>
		>();
	}
	HScene Param::makeFirstScene() const {
		tls_shared = UserShare();
		return rev_mgr_obj.emplace<glTF2Scene>();
	}
	Param::~Param() {
		tls_shared.terminate();
	}
}
