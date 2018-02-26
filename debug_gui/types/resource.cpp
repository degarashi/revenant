#include "../../handle.hpp"
#include "../../camera2d.hpp"
#include "../../imgui/imgui.h"
#include "../print.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			#define DEF_BEGIN if(false) {}
			#define DEF_(name, func, c)	else if(auto* p = dynamic_cast<c ::rev::name*>(&r)) { return func(*p); }
			#define DEF_END else ImGui::Text("not implemented type (%s)", typeid(r).name());

			#define DEF_SHOW(name)	DEF_(name, _Show, const)
			void _Show(const Resource& r) {
				DEF_BEGIN
				DEF_SHOW(Camera2D)
				DEF_END
			}

			#define DEF_EDIT(name)	DEF_(name, _Edit, )
			bool _Edit(Resource& r) {
				DEF_BEGIN
				DEF_EDIT(Camera2D)
				DEF_END
				return false;
			}
			#undef DEF_EDIT

			#define DEF_SLIDER(name)	DEF_(name, _Slider, )
			bool _Slider(Resource& r, ...) {
				DEF_BEGIN
				DEF_SLIDER(Camera2D)
				DEF_END
				return false;
			}
			#undef DEF_SLIDER
		}
	}
}
