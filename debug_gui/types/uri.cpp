#include "../print.hpp"
#include "../../uri.hpp"
#include "../../imgui/imgui.h"
#include "../entry.hpp"
#include "../style.hpp"
#include "../child.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			void _Show(const URI& u) {
				BranchURI(u, [](const auto& u){
					_Show(u);
				});
			}
			void _Show(const IdURI& u) {
				if(const auto child = ChildPush("IdURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
					Entry ent(0, 2);
					ent.show("id", u.getId());
				}
			}
			void _Show(const UserURI& u) {
				if(const auto child = ChildPush("UserURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
					Entry ent(0, 2);
					ent.show("name", u.getName());
				}
			}
			void _Show(const FileURI& u) {
				const auto style = MakeStylePush(
					ImGuiStyleVar_WindowPadding, ImVec2{0.f, 0.f}
				);
				if(const auto child = ChildPush("FileURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
					Entry ent(0, 2);
					ent.show("path", u.pathblock().plain_utf8());
				}
			}
			void _Show(const DataURI& u) {
				if(const auto child = ChildPush("DataURI", {0, ImGui::GetTextLineHeightWithSpacing()}, false, ImGuiWindowFlags_NoScrollbar)) {
					Entry ent(0, 2);
					ent.show("base64", u._bBase64);
					ent.show("data", std::to_string(u._data.size()) + " bytes");
					ImGui::Separator();
					for(auto& m : u._mediaType) {
						ent.show(m.first, m.second);
					}
				}
			}
		}
	}
}
