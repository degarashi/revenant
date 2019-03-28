#pragma once
#include "debug_gui/if.hpp"
#ifdef DEBUGGUI_ENABLED
	#include "debug_gui/child.hpp"
	#include "lubee/src/arithmetic.hpp"
	#include "debug_gui/spacing.hpp"
#endif
#include <tuple>
#include <utility>

namespace rev {
	namespace interval {
		//! Nフレーム後に動作を再開する
		struct Wait : IDebugGui {
			int32_t		wait;
			Wait(int32_t n);
			bool advance();

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
		};
		//! Nフレーム毎に動作する
		struct EveryN : IDebugGui {
			uint32_t	nth,
						cur;
			EveryN(uint32_t n);
			bool advance();

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
		};
		struct Accum : IDebugGui {
			uint32_t	accum = 0,
						skipped = 0;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
		};

		//! 複数のインターバルクラスを直列に繋げる
		template <int N>
		using IConst = std::integral_constant<int,N>;
		template <class... Ts>
		struct Combine : std::tuple<Ts...>, IDebugGui {
			Accum		ac;

			using std::tuple<Ts...>::tuple;
			bool _advance(IConst<sizeof...(Ts)>) {
				return true;
			}
			template <int N>
			bool _advance(IConst<N>) {
				if(std::get<N>(*this).advance())
					return _advance(IConst<N+1>());
				return false;
			}
			bool advance() {
				if(_advance(IConst<0>())) {
					++ac.accum;
					return true;
				}
				++ac.skipped;
				return false;
			}
			#ifdef DEBUGGUI_ENABLED
				const char* getDebugName() const noexcept override {
					return "Combined Interval";
				}
				template <std::size_t... Idx>
				bool _property(const bool edit, std::index_sequence<Idx...>) {
					return lubee::Or_A(std::get<Idx>(*this).property(edit)...);
				}
				bool property(bool edit) override {
					bool ret = _property(edit, std::make_index_sequence<sizeof...(Ts)>{});
					ret |= ac.property(edit);
					return ret;
				}
			#endif
		};
	}
}
