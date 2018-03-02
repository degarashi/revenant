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
		template <class M>
		class ResMgrC {
			private:
				const M&	_m;
			public:
				ResMgrC(const M& m):
					_m(m)
				{}
				void show() const {
					inner::ResMgr_Iter(_m, [](auto&& r){
						Show("", r);
					});
				}
		};
		template <class M>
		class ResMgr : public ResMgrC<M> {
			private:
				M&	_m;
			public:
				ResMgr(M& m):
					ResMgrC<M>(m),
					_m(m)
				{}
				bool edit() const {
					bool ret = false;
					inner::ResMgr_Iter(_m, [&ret](auto&& r){
						ret |= Edit("", r);
					});
					return ret;
				}
		};
		namespace inner {
			template <class T, class A>
			void _Show(const spi::ResMgr<T,A>& m) {
				ResMgrC<spi::ResMgr<T,A>>(m).show();
			}
			template <class T, class A>
			bool _Edit(spi::ResMgr<T,A>& m) {
				return ResMgr<spi::ResMgr<T,A>>(m).edit();
			}
		}
	}
}
