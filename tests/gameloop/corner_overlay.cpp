#ifdef DEBUGGUI_ENABLED
#include "corner_overlay.hpp"
#include "../../imgui/imgui.h"

namespace rev {
	int CornerOverlay::applyWindowPos() const {
		const int p_i = static_cast<int>(pivot);
		auto& io = ImGui::GetIO();
		const ImVec2
			pos{
				(p_i & 1) ? io.DisplaySize.x - dist.x : dist.x,
				(p_i & 2) ? io.DisplaySize.y - dist.y : dist.y
			},
			pv {
				(p_i & 1) ? 1.f : 0.f,
				(p_i & 2) ? 1.f : 0.f
			};
		ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pv);
		ImGui::SetNextWindowBgAlpha(alpha);
		return ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing;
	}
}
#endif
