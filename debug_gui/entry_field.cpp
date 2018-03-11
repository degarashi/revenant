#include "entry_field.hpp"
#include "../imgui/imgui.h"
#include "spacing.hpp"

namespace rev {
	namespace debug {
		EntryField::EntryField(const char* label, const float height, const int columns):
			ChildPush(label, {0, (height > 0) ? height+Spacing::Child() : 0}, true, 0),
			Entry(0, columns)
		{
			ImGui::SetColumnWidth(0, 120.f);
		}
	}
}
