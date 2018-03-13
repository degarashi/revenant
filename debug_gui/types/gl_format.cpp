#include "../../gl_format.hpp"
#include "../../imgui/imgui.h"
#include "../entry_field.hpp"

namespace rev {
	namespace {
		void GetDim(const uint32_t) {}
		template <class... Ts>
		void GetDim(const uint32_t n, int& d0, Ts&... dst) {
			d0 = n&0x07;
			GetDim(n>>3, dst...);
		}
	}
	void GLSLFormatDesc::showAsRow() const {
		debug::Show(nullptr, type);
		ImGui::NextColumn();

		int dx, dy;
		GetDim(dim, dx, dy);
		if(dy > 0) {
			ImGui::Text("Dim: %dx%d", dy, dx);
		} else
			ImGui::Text("Dim: %d", dx);
		ImGui::NextColumn();

		debug::Show(nullptr, bUnsigned);
		ImGui::NextColumn();
		debug::Show(nullptr, bCubed);
		ImGui::NextColumn();
	}
}
