#include "beat/pose2d.hpp"
#include "debug_gui/types/vector.hpp"
#include "../../imgui/imgui.h"
#include "../indent.hpp"
#include "../entry_field.hpp"
#include "../spacing.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			namespace {
				const char	*lb_pose2d = "Pose2D",
							*lb_offset = "Offset",
							*lb_scale = "Scale",
							*lb_rotation = "Rotation";
			}
			void _Show(const ::beat::g2::Pose& p) {
				auto f = EntryField(lb_pose2d);
				f.show(lb_offset, p.getOffset());
				f.show(lb_scale, p.getScaling());
				f.show(lb_rotation, p.getRotation());
			}
			bool _Edit(::beat::g2::Pose& p) {
				auto f = EntryField(lb_pose2d);
				{
					auto tmp = p.getOffset();
					if(f.edit(lb_offset, tmp)) {
						p.setOffset(tmp);
					}
				}
				{
					auto tmp = p.getScaling();
					if(f.edit(lb_scale, tmp)) {
						p.setScaling(tmp);
					}
				}
				{
					auto tmp = p.getRotation();
					if(f.edit(lb_rotation, tmp)) {
						p.setRotation(tmp);
					}
				}
				return f.modified();
			}
		}
	}
}
