#include "pose2d.hpp"
#include "camera2d.hpp"
#include "../../camera2d.hpp"
#include "../../imgui/imgui.h"
#include "../constant.hpp"
#include "lubee/compare.hpp"
#include "../entry.hpp"
#include "../group.hpp"

namespace rev {
	namespace debug {
		namespace {
			const char	*lb_pose = "Pose",
						*lb_aspect = "Aspect",
						*lb_accum = "Accum";
		}
		// ----------------------- Camera2DC -----------------------
		Camera2DC::Camera2DC(const ::rev::Camera2D& c):
			_c(c)
		{}
		void Camera2DC::show() const {
			const GroupPush group;
			Entry ent(0, 2);
			ent.show(lb_pose, _c.getPose());
			ent.show(lb_aspect, _c.getAspectRatio());
			ent.show(lb_accum, _c.getAccum()._value);
		}

		// ----------------------- Camera2D -----------------------
		Camera2D::Camera2D(::rev::Camera2D& c):
			Camera2DC(c),
			_c(c)
		{}
		bool Camera2D::edit() const {
			const GroupPush group;
			Entry ent(0, 2);
			ent.edit(lb_pose, _c._rflag.ref<::rev::Camera2D::Pose>());
			ent.edit(lb_aspect, _c._rflag.ref<::rev::Camera2D::AspectRatio>());
			ent.edit(lb_accum, _c._rflag.ref<::rev::Camera2D::Accum>());
			return ent.modified();
		}

		namespace inner {
			void _Show(const ::rev::Camera2D& c) {
				Camera2DC(c).show();
			}
			bool _Edit(::rev::Camera2D& c) {
				return Camera2D(c).edit();
			}
		}
	}
}
