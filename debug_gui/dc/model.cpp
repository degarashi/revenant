#include "../indent.hpp"
#include "../id.hpp"
#include "../../dc/model.hpp"
#include "../../dc/node.hpp"
#include "../../imgui/imgui.h"
#include "../../dc/mesh.hpp"
#include "../listview.hpp"

namespace rev::dc {
	const char* Model::getDebugName() const noexcept {
		return "Model";
	}
	bool Model::property(const bool edit) {
		bool mod = false;
		static int num = 0;
		ImGui::RadioButton("Node", &num, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Mesh", &num, 1);
		ImGui::Separator();
		if(num == 0) {
			const auto _ = debug::IndentPush();
			const auto __ = debug::IdPush("Node");
			mod |= _tf->property(edit);
		} else {
			debug::ListView(_mesh.cbegin(), _mesh.cend(), true);
		}
		return mod;
	}
}
