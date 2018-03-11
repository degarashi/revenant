#include "spacing.hpp"
#include "../imgui/imgui.h"
#include <algorithm>

namespace rev {
	namespace debug {
		float Spacing::Text() {
			return ImGui::GetTextLineHeight();
		}
		float Spacing::Separator() {
			return ItemSpacing();
		}
		float Spacing::ItemSpacing() {
			const auto& s = ImGui::GetStyle();
			return s.ItemSpacing.y;
		}
		float Spacing::Item() {
			return ImGui::GetItemsLineHeightWithSpacing() - ItemSpacing();
		}
		float Spacing::ItemText() {
			return std::max(Item(), Text());
		}
		float Spacing::Child() {
			const auto& s = ImGui::GetStyle();
			return s.WindowPadding.y * 2;
		}
	}
}
