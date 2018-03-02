#include "pose2d.hpp"
#include "../../camera2d.hpp"
#include "../../imgui/imgui.h"
#include "../entry_field.hpp"

namespace rev {
	namespace {
		const char	*lb_pose = "Pose",
					*lb_aspect = "Aspect",
					*lb_accum = "Accum";
	}
	bool Camera2D::guiEditor(bool) {
		if(auto field = debug::EntryField("Camera2D")) {
			if(field.edit(lb_pose, _rflag.ref<Pose>())) {
				refPose();
			}
			if(field.edit(lb_aspect, _rflag.ref<AspectRatio>())) {
				setAspectRatio(getAspectRatio());
			}
			field.show(lb_accum, uint64_t(getAccum()));
			return field.modified();
		}
		return false;
	}
}
