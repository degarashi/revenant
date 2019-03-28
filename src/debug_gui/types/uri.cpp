#include "../print.hpp"
#include "../../uri/uri.hpp"
#include "../../imgui/imgui.h"
#include "../entry.hpp"
#include "../style.hpp"
#include "../child.hpp"

namespace rev {
	std::string URI::summary_str() const {
		std::stringstream ss;
		ss << *this;
		return ss.str();
	}
	void URI::summary() const {
		ImGui::TextUnformatted(summary_str().c_str());
	}

	bool IdURI::property(const bool edit) {
		if(const auto child = debug::ChildPush("IdURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, edit, 2);
			ent.entry("id", _num);
			return ent.modified();
		}
		return false;
	}
	const char* IdURI::getDebugName() const noexcept {
		return "URI(Id)";
	}
	bool UserURI::property(const bool edit) {
		if(const auto child = debug::ChildPush("UserURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
			debug::Entry ent(0, edit, 2);
			ent.show("name", getName());
			return ent.modified();
		}
		return false;
	}
	const char* UserURI::getDebugName() const noexcept {
		return "URI(User)";
	}
	bool FileURI::property(const bool edit) {
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
	const char* FileURI::getDebugName() const noexcept {
		return "URI(File)";
	}
	bool DataURI::property(const bool edit) {
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
	const char* DataURI::getDebugName() const noexcept {
		return "URI(Data)";
	}
}
