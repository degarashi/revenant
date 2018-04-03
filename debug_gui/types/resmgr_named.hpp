#pragma once
#include "spine/resmgr_named.hpp"
#include "../print.hpp"
#include "../id.hpp"
#include "../column.hpp"
#include "../child.hpp"
#include "../tree.hpp"
#include "../state_storage.hpp"
#include "../textfilter.hpp"
#include "../sstream.hpp"
#include "../popup.hpp"
#include "../resource_window.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			template <class M, class CB>
			void ResMgrNamed_Iter(M& m, CB&& cb) {
				StringStream s;
				TextFilter filter(ImGui::GetID("filter"));
				const auto print = [&m, &s, &filter, &cb](const bool anonymous){
					int id = 0;
					for(const auto& r : m) {
						const auto& key = *m.getKey(r);
						if(anonymous == spi::IsAnonymous(key)) {
							s << r->summary_str() << ": " << key;
							const auto ks = s.output();
							const auto* kp = ks.c_str();
							if(filter.PassFilter(kp)) {
								const IdPush idp(++id);
								cb(kp, r);
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
				};
				if(const auto c = ChildPush("ResMgrNamed", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
					if(const auto t = TreePush("Named")) {
						print(false);
					}
					if(const auto t = TreePush("Anonymous")) {
						print(true);
					}
				}
			}
			template <class T, class K, class A>
			bool _Edit(spi::ResMgrName<T,K,A>& m) {
				const auto c = ColumnPush(2);
				const auto id = ImGui::GetID("Left");
				using R = typename std::decay_t<decltype(*m.begin())>::element_type;
				using St = StateStorage<std::weak_ptr<IDebugGui>>;
				auto cur_lk = St::template Get<R>(id);
				inner::ResMgrNamed_Iter(m, [id, &cur_lk](const char* name, auto&& r){
					if(ImGui::Selectable(name, cur_lk == r)) {
						St::Set(id, r);
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
