#pragma once
#include "../print.hpp"
#include "spine/resmgr.hpp"
#include "../../imgui/imgui.h"
#include "../column.hpp"
#include "../child.hpp"
#include "../state_storage_res.hpp"
#include "../sstream.hpp"
#include "../popup.hpp"
#include "../resource_window.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			template <class M, class CB>
			void ResMgr_Iter(M& m, CB&& cb) {
				StringStream s;
				if(const auto c = ChildPush("ResMgr", 0, false, ImGuiWindowFlags_HorizontalScrollbar)) {
					for(auto&& r : m) {
						const IdPush id(r.get());
						s << r->getDebugName() << "\t(address: 0x" << std::hex << r.get() << ')';
						cb(s.output().c_str(), r);
						if(ImGui::IsItemClicked(1)) {
							ImGui::OpenPopup("popup");
						}
						if(const auto _ = debug::PopupPush("popup")) {
							if(ImGui::Selectable("Open in new window...")) {
								debug::ResourceWindow::Add(r);
							}
						}
					}
				}
			}
			template <class T, class A>
			bool _Edit(spi::ResMgr<T,A>& m) {
				const auto c = ColumnPush(2);
				const auto id = ImGui::GetID("ResMgr");
				using R = typename std::decay_t<decltype(*m.begin())>::element_type;
				auto cur_lk = StateStorage_Res::template Get<R>(id);
				inner::ResMgr_Iter(m, [id, &cur_lk](const char* name, auto&& r){
					if(ImGui::Selectable(name, cur_lk == r)) {
						StateStorage_Res::Set(id, r);
						cur_lk = r;
					}
				});

				ImGui::NextColumn();
				if(cur_lk) {
					if(const auto _ = ChildPush("Right", 0, false, ImGuiWindowFlags_HorizontalScrollbar)) {
						return Edit("", cur_lk);
					}
				}
				return false;
			}
		}
	}
}
