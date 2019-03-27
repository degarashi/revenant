#include "../../gl/format.hpp"
#include "../../imgui/imgui.h"
#include "../entry_field.hpp"

namespace rev {
	void GLSLFormatDesc::showAsRow() const {
		debug::Show(nullptr, type);
		ImGui::NextColumn();

		uint32_t dx, dy;
		DecompDim(dim, dx, dy);
		if(dy > 0) {
			ImGui::Text("%dx%d", dy, dx);
		} else
			ImGui::Text("%d", dx);
		ImGui::NextColumn();

		debug::Show(nullptr, bUnsigned);
		ImGui::NextColumn();
		debug::Show(nullptr, bCubed);
		ImGui::NextColumn();
	}
}
