#include "../../camera2d.hpp"
#include "../../imgui/imgui.h"
#include "../entry_field.hpp"

namespace rev {
	bool Camera2D::guiEditor(const bool edit) {
		auto field = debug::EntryField("Camera2D");
		ImGui::Columns(1);
		if(field.entry(edit, "pose", _rflag.ref<Pose>())) {
			refPose();
		}
		ImGui::Columns(2);
		if(field.entry(edit, "Aspect", _rflag.ref<AspectRatio>())) {
			setAspectRatio(getAspectRatio());
		}
		field.show("Accum", uint64_t(getAccum()));
		return field.modified();
	}
}
