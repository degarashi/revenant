#include "group.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		GroupPush::GroupPush() {
			ImGui::BeginGroup();
		}
		GroupPush::~GroupPush() {
			ImGui::EndGroup();
		}
	}
}
