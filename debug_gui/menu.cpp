#include "menu.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		MenuPush::MenuPush(const char* label, const bool enabled) {
			_valid = ImGui::BeginMenu(label, enabled);
		}
		MenuPush::MenuPush(MenuPush&& m):
			_valid(m._valid)
		{
			m._valid = false;
		}
		MenuPush::operator bool () const noexcept {
			return _valid;
		}
		MenuPush::~MenuPush() {
			if(_valid)
				ImGui::EndMenu();
		}
	}
}
