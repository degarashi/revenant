#include "entry_field.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		FieldLabel::FieldLabel(const char* label) {
			ImGui::TextUnformatted(label);
			ImGui::Separator();
		}

		EntryField::EntryField(const char* label, const int n, const int initial, const int columns):
			StylePush<1>(ImGuiStyleVar_WindowPadding, ImVec2{4.f, 4.f}),
			ChildPush(label, {0, (ImGui::GetTextLineHeightWithSpacing()+4)*(n+2)}, true, 0),
			FieldLabel(label),
			Entry(initial, columns)
		{}
	}
}
