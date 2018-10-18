#include "../../camera3d.hpp"
#include "../entry_field.hpp"

namespace rev {
	const char* Camera3D::getDebugName() const noexcept {
		return "Camera3D";
	}
	bool Camera3D::property(const bool edit) {
		auto f = debug::EntryField("Camera3D", edit);
		ImGui::Columns(1);
		if(f.entry("pose", _rflag.ref<Pose>()))
			refPose();
		ImGui::Columns(2);
		if(f.entry("Fov", _rflag.ref<Fov>()))
			refFov();
		if(f.entry("Aspect", _rflag.ref<Aspect>()))
			refAspect();
		if(f.entry("NearZ", _rflag.ref<NearZ>()))
			refNearZ();
		if(f.entry("FarZ", _rflag.ref<FarZ>()))
			refFarZ();
		f.show("Accum", uint64_t(getAccum()));
		auto& rot = getPose().getRotation();
		f.show("Right", rot.getRight());
		f.show("Up", rot.getUp());
		f.show("Dir", rot.getDir());
		return f.modified();
	}
}
