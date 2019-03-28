#include "resmgr_app.hpp"
#include "param.hpp"
#include "sprite_scene.hpp"
#include "../../gl/resource.hpp"
#include "../../drawutil/gle_nest.hpp"
#include "../../effect/sys_uniform.hpp"
#include "../../effect/u_matrix2d.hpp"
#include "../../effect/u_common.hpp"

namespace rev::test {
	HFx Param::makeEffect() const {
		return mgr_gl.template makeResource<
			drawutil::GLE_Nest<
				SystemUniform,
				U_Matrix2D,
				U_Common,
				GLEffect
			>
		>();
	}
	HScene Param::makeFirstScene() const {
		return rev_mgr_obj.emplace<SpriteScene>();
	}
	Param::~Param() {}
}
