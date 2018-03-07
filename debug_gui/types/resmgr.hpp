#pragma once
#include "../print.hpp"
#include "spine/resmgr.hpp"
#include "../../imgui/imgui.h"
#include "../column.hpp"
#include "../child.hpp"
#include <unordered_map>

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
			using SelectPos = std::unordered_map<ImGuiID, std::weak_ptr<void>>;
			extern SelectPos g_selectPos;

			template <class T, class A>
			bool _Edit(spi::ResMgr<T,A>& m) {
				const auto c = ColumnPush(2);
				const auto id = ImGui::GetID("ResMgr");
				auto& cur = g_selectPos[id];
				const auto cur_lk = cur.lock();
				inner::ResMgr_Iter(m, [&cur, &cur_lk](const char* name, auto&& r){
					if(ImGui::Selectable(name, cur_lk == r))
						cur = r;
				});

				ImGui::NextColumn();
				if(const auto sp = cur.lock()) {
					using R = typename std::decay_t<decltype(*m.begin())>::element_type;
					return Edit("", *reinterpret_cast<R*>(sp.get()));
				}
				return false;
			}
		}
	}
}
