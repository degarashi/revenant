#include "../print.hpp"
#include "../../uri.hpp"
#include "../../imgui/imgui.h"
#include "../entry.hpp"
#include "../style.hpp"
#include "../child.hpp"

namespace rev {
	void IdURI::guiViewer(bool) {
		if(const auto child = debug::ChildPush("IdURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, 2);
			ent.show("id", getId());
		}
	}
	void UserURI::guiViewer(bool) {
		if(const auto child = debug::ChildPush("UserURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, 2);
			ent.show("name", getName());
		}
	}
	void FileURI::guiViewer(bool) {
		const auto style = debug::MakeStylePush(
			ImGuiStyleVar_WindowPadding, ImVec2{0.f, 0.f}
		);
		if(const auto child = debug::ChildPush("FileURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, 2);
			ent.show("path", pathblock().plain_utf8());
		}
	}
	void DataURI::guiViewer(bool) {
		if(const auto child = debug::ChildPush("DataURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, 2);
			ent.show("base64", _bBase64);
			ent.show("data", std::to_string(_data.size()) + " bytes");
			ImGui::Separator();
			for(auto& m : _mediaType) {
				ent.show(m.first, m.second);
			}
		}
	}
}
