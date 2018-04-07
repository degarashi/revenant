#include "tree.hpp"
#include "imgui/imgui.h"

namespace rev {
	namespace debug {
		TreePush::TreePush(const char* label, const ImGuiTreeNodeFlags flag) {
			_valid = ImGui::TreeNodeEx(label, flag);
			if(flag & ImGuiTreeNodeFlags_NoTreePushOnOpen)
				_valid = false;
		}
		TreePush::TreePush(TreePush&& t):
			_valid(t._valid)
		{
			t._valid = false;
		}
		TreePush::~TreePush() {
			if(_valid)
				ImGui::TreePop();
		}
		TreePush::operator bool () const noexcept {
			return _valid;
		}
	}
}
