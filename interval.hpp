#pragma once
#include <tuple>
#include <utility>
#ifdef DEBUGGUI_ENABLED
	#include "debug_gui/child.hpp"
	#include "lubee/arithmetic.hpp"
#endif

namespace rev {
	namespace interval {
		//! Nフレーム後に動作を再開する
		struct Wait {
			int32_t		wait;
			Wait(int32_t n);
			bool advance();
			#ifdef DEBUGGUI_ENABLED
				bool guiEditor();
				float guiHeight() const;
			#endif
		};
		//! Nフレーム毎に動作する
		struct EveryN {
			uint32_t	nth,
						cur;
			EveryN(uint32_t n);
			bool advance();
			#ifdef DEBUGGUI_ENABLED
				bool guiEditor();
				float guiHeight() const;
			#endif
		};
		struct Accum {
			uint32_t	accum = 0,
						skipped = 0;

			#ifdef DEBUGGUI_ENABLED
				void guiViewer() const;
				float guiHeight() const;
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
				bool _guiEditor(std::index_sequence<Idx...>) {
					return lubee::Or_A(std::get<Idx>(*this).guiEditor()...);
				}
				template <std::size_t... Idx>
				float _guiHeight(std::index_sequence<Idx...>) const {
					return lubee::Add(std::get<Idx>(*this).guiHeight()...) + ac.guiHeight();
				}
				bool guiEditor() {
					const float w = _guiHeight(std::make_index_sequence<sizeof...(Ts)>{});
					if(const auto c = debug::ChildPush("Combine", w, true)) {
						const bool ret = _guiEditor(std::make_index_sequence<sizeof...(Ts)>{});
						ac.guiViewer();
						return ret;
					}
					return false;
				}
			#endif
		};
	}
}
