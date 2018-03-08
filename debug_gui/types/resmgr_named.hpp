#pragma once
#include "spine/resmgr_named.hpp"
#include "../print.hpp"
#include "../../imgui/imgui.h"
#include "../id.hpp"
#include "../column.hpp"
#include "../child.hpp"
#include "../tree.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			template <class M, class CB>
			void ResMgrNamed_Iter(M& m, CB&& cb) {
				std::stringstream ss;
				static char cbuff[128];
				ImGui::InputText("filter", cbuff, 128);
				ImGuiTextFilter filter(cbuff);
				const auto print = [&m, &ss, &filter, &cb](const bool anonymous){
					const auto clear = [&ss](){
						ss.clear();
						ss.str("");
					};
					int id = 0;
					for(const auto& r : m) {
						const auto& key = *m.getKey(r);
						ss << r->getDebugName() << ": " << key;
						if(anonymous == spi::IsAnonymous(key)) {
							const auto ks = ss.str();
							const auto* kp = ks.c_str();
							if(filter.PassFilter(kp)) {
								const IdPush idp(++id);
								cb(kp, r);
							}
						}
						clear();
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
			using SelectPos = std::unordered_map<ImGuiID, std::weak_ptr<void>>;
			extern SelectPos g_selectPos;

			template <class T, class K, class A>
			bool _Edit(spi::ResMgrName<T,K,A>& m) {
				const auto c = ColumnPush(2);
				const auto id = ImGui::GetID("Left");
				auto& cur = g_selectPos[id];
				const auto cur_lk = cur.lock();
				inner::ResMgrNamed_Iter(m, [&cur, &cur_lk](const char* name, auto&& r){
					if(ImGui::Selectable(name, cur_lk == r))
						cur = r;
				});

				ImGui::NextColumn();
				if(const auto sp = cur.lock()) {
					using R = typename std::decay_t<decltype(*m.begin())>::element_type;
					return Edit("", *std::static_pointer_cast<R>(sp));
				}
				return false;
			}
		}
	}
}
