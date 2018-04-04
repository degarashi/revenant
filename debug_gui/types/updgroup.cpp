#include "../../updgroup.hpp"
#include "../state_storage.hpp"
#include "../column.hpp"
#include "../id.hpp"
#include "../print.hpp"
#include "../popup.hpp"
#include "../textfilter.hpp"
#include "../sstream.hpp"
#include "../resource_window.hpp"
#include "../tree.hpp"
#include <iomanip>

namespace rev {
	bool UpdGroup::property(const bool edit) {
		bool mod = false;
		if(const auto _ = debug::TreePush(IObject::getDebugName())) {
			mod |= IObject::property(edit);
		}
		if(const auto _ = debug::TreePush("Interval")) {
			mod |= _interval.property(edit);
		}
		ImGui::Separator();
		{
			const auto col = debug::ColumnPush(2);
			const auto id_obj = ImGui::GetID("Object");
			using St = debug::StateStorage<WDbg>;
			auto cur = St::Get<IObject>(id_obj);
			if(const auto c = debug::ChildPush("Left", {0,0})) {
				debug::TextFilter filter(ImGui::GetID("Filter"));
				const float h = ImGui::GetContentRegionAvail().y;
				ImGui::TextUnformatted("Member");
				StringStream s;
				s << std::hex;
				const auto makename = [&s](const auto& p) {
					s << "0x" << std::setfill('0') << std::setw(8);
					if(p.second->isNode())
						s << " [G]";
					if(p.second->isDead())
						s << " [D]";
					s << p.first << "\t" << p.second->getDebugName();
					return s.output();
				};
				if(const auto c = debug::ChildPush("Member", {0, h*0.75f}, true, ImGuiWindowFlags_HorizontalScrollbar)) {
					int idx = 0;
					for(auto& p : _objV) {
						const auto idp = debug::IdPush(idx++);
						const auto str = makename(p);
						if(filter.PassFilter(str.c_str())) {
							if(ImGui::Selectable(str.c_str(), cur == p.second)) {
								cur = p.second;
								St::Set(id_obj, cur);
							}
							if(ImGui::IsItemClicked(1)) {
								ImGui::OpenPopup("popup");
							}
							if(const auto pu = debug::PopupPush("popup")) {
								if(ImGui::Selectable("Open in new window...")) {
									debug::ResourceWindow::Add(p.second);
								}
								if(ImGui::Selectable("Delete")) {
									remObj(p.second);
								}
							}
						}
					}
				}
				ImGui::TextUnformatted("Add(Reserved)");
				if(const auto c = debug::ChildPush("Add", {0, 0}, true, ImGuiWindowFlags_HorizontalScrollbar)) {
					int idx = 0;
					for(auto& p : _addObj) {
						const auto idp = debug::IdPush(idx++);
						const auto str = makename(p);
						if(ImGui::Selectable(str.c_str(), cur == p.second)) {
							cur = p.second;
							St::Set(id_obj, cur);
						}
					}
				}
			}
			ImGui::NextColumn();
			if(cur) {
				if(const auto _ = debug::ChildPush("Right", 0, true, ImGuiWindowFlags_HorizontalScrollbar)) {
					mod |= debug::Edit("", cur);
				}
			}
		}
		return mod;
	}
}
