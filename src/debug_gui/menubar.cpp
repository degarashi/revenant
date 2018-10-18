#include "menubar.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		MenuBarPush::MenuBarPush() {
			_valid = ImGui::BeginMenuBar();
		}
		MenuBarPush::MenuBarPush(MenuBarPush&& m):
			_valid(m._valid)
		{
			m._valid = false;
		}
		MenuBarPush::operator bool () const noexcept {
			return _valid;
		}
		MenuBarPush::~MenuBarPush() {
			if(_valid)
				ImGui::EndMenuBar();
		}
	}
}
