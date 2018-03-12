#pragma once
#include <tuple>
#include <utility>
#ifdef DEBUGGUI_ENABLED
	#include "debug_gui/child.hpp"
	#include "lubee/arithmetic.hpp"
	#include "debug_gui/spacing.hpp"
#endif

namespace rev {
	namespace interval {
		//! Nフレーム後に動作を再開する
		struct Wait {
			int32_t		wait;
			Wait(int32_t n);
			bool advance();
			#ifdef DEBUGGUI_ENABLED
				bool guiEditor(bool edit);
			#endif
		};
		//! Nフレーム毎に動作する
		struct EveryN {
			uint32_t	nth,
						cur;
			EveryN(uint32_t n);
			bool advance();
			#ifdef DEBUGGUI_ENABLED
				bool guiEditor(bool edit);
			#endif
		};
		struct Accum {
			uint32_t	accum = 0,
						skipped = 0;

			#ifdef DEBUGGUI_ENABLED
				bool guiEditor(bool edit);
			#endif
		};

		//! 複数のインターバルクラスを直列に繋げる
		template <int N>
		using IConst = std::integral_constant<int,N>;
		template <class... Ts>
		struct Combine : std::tuple<Ts...> {
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
				template <std::size_t... Idx>
				bool _guiEditor(const bool edit, std::index_sequence<Idx...>) {
					return lubee::Or_A(std::get<Idx>(*this).guiEditor(edit)...);
				}
				bool guiEditor(bool edit) {
					bool ret = _guiEditor(edit, std::make_index_sequence<sizeof...(Ts)>{});
					ret |= ac.guiEditor(edit);
					return ret;
				}
			#endif
		};
	}
}
