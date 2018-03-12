#include "../print.hpp"
#include "../../uri.hpp"
#include "../../imgui/imgui.h"
#include "../entry.hpp"
#include "../style.hpp"
#include "../child.hpp"

namespace rev {
	bool IdURI::guiEditor(const bool edit) {
		if(const auto child = debug::ChildPush("IdURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, edit, 2);
			ent.show("id", getId());
			return ent.modified();
		}
		return false;
	}
	bool UserURI::guiEditor(const bool edit) {
		if(const auto child = debug::ChildPush("UserURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, edit, 2);
			ent.show("name", getName());
			return ent.modified();
		}
		return false;
	}
	bool FileURI::guiEditor(const bool edit) {
		const auto style = debug::StylePush(
			ImGuiStyleVar_WindowPadding, ImVec2{0.f, 0.f}
		);
		if(const auto child = debug::ChildPush("FileURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, edit, 2);
			ent.show("path", pathblock().plain_utf8());
			return ent.modified();
		}
		return false;
	}
	bool DataURI::guiEditor(const bool edit) {
		if(const auto child = debug::ChildPush("DataURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, edit, 2);
			ent.show("base64", _bBase64);
			ent.show("data", std::to_string(_data.size()) + " bytes");
			ImGui::Separator();
			for(auto& m : _mediaType) {
				ent.show(m.first, m.second);
			}
			return ent.modified();
		}
		return false;
	}
}
