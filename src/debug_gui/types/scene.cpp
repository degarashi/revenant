#include "../../object/scene/if.hpp"
#include "../../imgui/imgui.h"
#include "../print.hpp"
#include "../child.hpp"
#include "../../object/drawgroup.hpp"

namespace rev {
	const char* IScene::getDebugName() const noexcept {
		return "IScene";
	}
	bool IScene::property(const bool edit) {
		if(const auto c = debug::ChildPush(IScene::getDebugName(), 0)) {
			auto* storage = ImGui::GetStateStorage();
			int* select = storage->GetIntRef(ImGui::GetID(""), 0);
			ImGui::RadioButton("Update", select, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Draw", select, 1);
			ImGui::Spacing();
			if(*select == 0) {
				auto tmp = getUpdGroup();
				debug::EditIf(edit, nullptr, tmp);
			} else if(*select == 1) {
				auto tmp = getDrawGroup();
				debug::EditIf(edit, nullptr, tmp);
			}
		}
		return false;
	}
}
