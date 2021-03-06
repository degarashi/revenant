#include "../../camera2d.hpp"
#include "../../imgui/imgui.h"
#include "../entry_field.hpp"

namespace rev {
	bool Camera2D::property(const bool edit) {
		auto field = debug::EntryField("Camera2D", edit);
		ImGui::Columns(1);
		if(field.entry("pose", _rflag.ref<Pose>())) {
			refPose();
		}
		ImGui::Columns(2);
		if(field.entry("Aspect", _rflag.ref<AspectRatio>())) {
			setAspectRatio(getAspectRatio());
		}
		field.show("Accum", uint64_t(getAccum()));
		return field.modified();
	}
	const char* Camera2D::getDebugName() const noexcept {
		return "Camera2D";
	}
}
