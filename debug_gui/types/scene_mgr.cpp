#include "../../scene_mgr.hpp"
#include "../../scene_if.hpp"
#include "../id.hpp"
#include "../child.hpp"
#include "../column.hpp"
#include "../state_storage_res.hpp"
#include "../print.hpp"
#include "../../imgui/imgui.h"

namespace rev {
	bool SceneMgr::property(bool) {
		if(const auto c = debug::ChildPush("SceneMgr", {0,0})) {
			const auto colmn = debug::ColumnPush(2);
			ImGui::SetColumnWidth(0, 120);
			const auto id = ImGui::GetID("");
			auto cur = debug::StateStorage_Res::Get<IScene>(id);
			{
				const auto idp = debug::IdPush("Left");
				ImGui::TextUnformatted("SceneStack");
				ImGui::Separator();
				int idx = 0;
				for(const auto& s : _scene) {
					const auto str = lubee::log::MakeMessage("[%d] %s", idx, s->getDebugName());
					if(ImGui::Selectable(str.c_str(), cur == s)) {
						cur = s;
						debug::StateStorage_Res::Set(id, cur);
					}
					++idx;
				}
			}
			ImGui::NextColumn();
			{
				const auto idp = debug::IdPush("Right");
				if(cur) {
					debug::Edit("", cur);
				}
			}
		}
		return false;
	}
}
