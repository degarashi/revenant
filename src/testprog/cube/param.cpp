#include "resmgr_app.hpp"
#include "param.hpp"
#include "cube_scene.hpp"
#include "../../gl/resource.hpp"
#include "../../util/gle_nest.hpp"
#include "../../sys_uniform.hpp"
#include "../../u_matrix2d.hpp"
#include "../../u_matrix3d.hpp"
#include "../../u_common.hpp"
#include "shared.hpp"

namespace rev::test {
	HFx Param::makeEffect() const {
		return mgr_gl.template makeResource<
			util::GLE_Nest<
				SystemUniform,
				U_Matrix2D,
				U_Matrix3D,
				U_Common,
				GLEffect
			>
		>();
	}
	HScene Param::makeFirstScene() const {
		tls_shared = UserShare();
		return rev_mgr_obj.emplace<CubeScene>();
	}
	Param::~Param() {
		tls_shared.terminate();
	}
}
