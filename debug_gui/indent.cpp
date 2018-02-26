#include "indent.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		IndentPush::IndentPush(const float w) {
			ImGui::Indent(w);
		}
		IndentPush::~IndentPush() {
			ImGui::Unindent();
		}
	}
}
