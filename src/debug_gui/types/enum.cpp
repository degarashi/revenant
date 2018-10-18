#include "../../imgui/imgui.h"
#include "spine/src/enum_t.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			bool _EditEnum(spi::Enum_t& value, const spi::Enum_t len, const char* (*func)(spi::Enum_t)) {
				const auto prev = value;
				bool ret = false;
				if(ImGui::BeginCombo("", func(value))) {
					for(spi::Enum_t i=0 ; i<len ; i++) {
						const bool sel = i == prev;
						if(ImGui::Selectable(func(i), sel))
							value = i;
						if(sel)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				return ret;
			}
		}
	}
}
