#pragma once
#include "spine/resmgr_named.hpp"
#include "../print.hpp"
#include "../../imgui/imgui.h"
#include "../id.hpp"
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
								if(const auto t = TreePush(kp)) {
									cb(*r);
								}
							}
						}
						clear();
					}
				};
				if(ImGui::CollapsingHeader("Named")) {
					print(false);
				}
				if(ImGui::CollapsingHeader("Anonymous")) {
					print(true);
				}
			}
		}
		namespace inner {
			template <class T, class K, class A>
			void _Show(const spi::ResMgrName<T,K,A>& m) {
				inner::ResMgrNamed_Iter(m, [](auto&& r){
					Show("", r);
				});
			}
			template <class T, class K, class A>
			bool _Edit(spi::ResMgrName<T,K,A>& m) {
				bool ret = false;
				inner::ResMgrNamed_Iter(m, [&ret](auto&& r){
					ret |= Edit("", r);
				});
				return ret;
			}
		}
	}
}
