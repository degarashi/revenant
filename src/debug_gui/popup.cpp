#include "popup.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		PopupPush::PopupPush(const char* id, ImGuiWindowFlags flags) {
			_valid = ImGui::BeginPopup(id, flags);
		}
		PopupPush::PopupPush(PopupPush&& m):
			_valid(m._valid)
		{
			m._valid = false;
		}
		PopupPush::operator bool () const noexcept {
			return _valid;
		}
		PopupPush::~PopupPush() {
			if(_valid)
				ImGui::EndPopup();
		}
	}
}
