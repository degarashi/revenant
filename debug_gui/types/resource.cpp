#include "../../handle.hpp"
#include "../../camera2d.hpp"
#include "../../imgui/imgui.h"

namespace rev {
	namespace debug {
		namespace inner {
			void _Show(const Resource& r) {
				if(const auto* p = dynamic_cast<const ::rev::Camera2D*>(&r))
					_Show(*p);
				else
					ImGui::Text("not implemented type (%s)", typeid(r).name());
			}
			bool _Edit(Resource& r) {
				if(auto* p = dynamic_cast<::rev::Camera2D*>(&r))
					return _Edit(*p);
				else
					ImGui::Text("not implemented type (%s)", typeid(r).name());
				return false;
			}
			bool _Slider(Resource& r, ...) {
				if(auto* p = dynamic_cast<::rev::Camera2D*>(&r))
					return _Slider(*p);
				else
					ImGui::Text("not implemented type (%s)", typeid(r).name());
				return false;
			}
		}
	}
}
