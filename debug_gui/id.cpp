#include "id.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		IdPush::IdPush(const int id) {
			ImGui::PushID(id);
		}
		IdPush::IdPush(const char* id) {
			ImGui::PushID(id);
		}
		IdPush::IdPush(const void* id) {
			ImGui::PushID(id);
		}
		IdPush::~IdPush() {
			ImGui::PopID();
		}
	}
}
