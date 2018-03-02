#include "pose2d.hpp"
#include "../../camera2d.hpp"
#include "../../imgui/imgui.h"
#include "../constant.hpp"
#include "lubee/compare.hpp"
#include "../entry_field.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			namespace {
				const char	*lb_pose = "Pose",
							*lb_aspect = "Aspect",
							*lb_accum = "Accum";
			}
			bool _Edit(::rev::Camera2D& c) {
				if(auto field = EntryField("Camera2D", 6, 0, 2)) {
					field.edit(lb_pose, c._rflag.template ref<::rev::Camera2D::Pose>());
					field.edit(lb_aspect, c._rflag.ref<::rev::Camera2D::AspectRatio>());
					field.show(lb_accum, uint64_t(c.getAccum()));
					return field.modified();
				}
				return false;
			}
		}
	}
}
