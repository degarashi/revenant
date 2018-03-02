#pragma once
#include "../print.hpp"
#include "spine/resmgr.hpp"
#include "../../imgui/imgui.h"
#include "../tree.hpp"

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
				for(auto&& r : m) {
					const IdPush id(r.get());
					ss << "address: 0x" << std::hex << r.get();
					if(const auto t = TreePush(ss.str().c_str())) {
						cb(*r);
					}
					clear();
				}
			}
		}
		namespace inner {
			template <class T, class A>
			void _Show(const spi::ResMgr<T,A>& m) {
				inner::ResMgr_Iter(m, [](auto&& r){
					Show("", r);
				});
			}
			template <class T, class A>
			bool _Edit(spi::ResMgr<T,A>& m) {
				bool ret = false;
				inner::ResMgr_Iter(m, [&ret](auto&& r){
					ret |= Edit("", r);
				});
				return ret;
			}
		}
	}
}
