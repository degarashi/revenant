#pragma once
#include "../print.hpp"
#include "spine/resmgr.hpp"
#include "../../imgui/imgui.h"
#include "../column.hpp"
#include "../child.hpp"
#include "../state_storage_res.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			template <class M, class CB>
			void ResMgr_Iter(M& m, CB&& cb) {
				std::stringstream ss;
				const auto clear = [&ss](){
					ss.str("");
					ss.clear();
				};
				if(const auto c = ChildPush("ResMgr", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
					for(auto&& r : m) {
						const IdPush id(r.get());
						ss << "address: 0x" << std::hex << r.get();
						cb(ss.str().c_str(), r);
						clear();
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
					return Edit("", cur_lk);
				}
				return false;
			}
		}
	}
}
