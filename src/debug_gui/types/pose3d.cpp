#include "beat/src/pose3d.hpp"
#include "../entry_field.hpp"

namespace rev::debug::inner {
	namespace {
		const char		*lb_pose3d = "Pose3D",
						*lb_offset = "Offset",
						*lb_rotation = "Rotation",
						*lb_scale = "Scale";
	}
	void _Show(const ::beat::g3::Pose& p) {
		auto f = EntryField(lb_pose3d, false);
		f.show(lb_offset, p.getOffset());
		f.show(lb_rotation, p.getRotation());
		f.show(lb_scale, p.getScaling());
	}
	bool _Edit(::beat::g3::Pose& p) {
		auto f = EntryField(lb_pose3d, true);
		{
			auto tmp = p.getOffset();
			if(f.edit(lb_offset, tmp)) {
				p.setOffset(tmp);
			}
		}
		{
			auto tmp = p.getRotation();
			if(f.edit(lb_rotation, tmp)) {
				p.setRotation(tmp);
			}
		}
		{
			auto tmp = p.getScaling();
			if(f.edit(lb_scale, tmp)) {
				p.setScaling(tmp);
			}
		}
		return f.modified();
	}
}
