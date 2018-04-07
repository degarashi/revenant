#include "header.hpp"
#include "tree.hpp"
#include "../imgui/imgui.h"

namespace rev::debug {
	ImGuiTreeNodeFlags Header::_GetFlag(const bool headerOnly, const bool defaultOpen) {
		ImGuiTreeNodeFlags flag = 0;
		if(headerOnly) {
			ImGui::Separator();
			flag |= ImGuiTreeNodeFlags_FramePadding |
					ImGuiTreeNodeFlags_Leaf |
					ImGuiTreeNodeFlags_NoTreePushOnOpen;
		} else
			flag |= ImGuiTreeNodeFlags_Framed;
		if(defaultOpen) {
			flag |= ImGuiTreeNodeFlags_DefaultOpen;
		}
		return flag;
	}
	const char* Header::_GetLabel(const char* label, const bool headerOnly) {
		if(headerOnly) {
			s_label = label;
			s_label.append(" (none)");
			return s_label.c_str();
		}
		return label;
	}
	Header::Header(const char*label, const bool headerOnly, const bool defaultOpen, const bool hideWhenEmpty) {
		if(headerOnly && hideWhenEmpty) {}
		else {
			_tree = std::make_shared<TreePush>(_GetLabel(label, headerOnly), _GetFlag(headerOnly, defaultOpen));
			if(headerOnly)
				ImGui::Separator();
		}
	}
	Header::operator bool () const noexcept {
		if(!_tree)
			return false;
		return static_cast<bool>(*_tree);
	}
}
