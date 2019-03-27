#include "resmgr_app.hpp"
#include "param.hpp"
#include "sprite_scene.hpp"
#include "../../gl/resource.hpp"
#include "../../util/gle_nest.hpp"
#include "../../sys_uniform.hpp"
#include "../../u_matrix2d.hpp"
#include "../../u_common.hpp"

namespace rev::test {
	HFx Param::makeEffect() const {
		return mgr_gl.template makeResource<
			util::GLE_Nest<
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
