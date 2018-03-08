#pragma once
#include <tuple>
#include <utility>

namespace rev {
	namespace interval {
		//! Nフレーム後に動作を再開する
		struct Wait {
			int32_t		wait;
			Wait(int32_t n);
			bool advance();
		};
		//! Nフレーム毎に動作する
		struct EveryN {
			uint32_t	nth,
						cur;
			EveryN(uint32_t n);
			bool advance();
		};

		//! 複数のインターバルクラスを直列に繋げる
		template <int N>
		using IConst = std::integral_constant<int,N>;
		template <class... Ts>
		struct Combine : std::tuple<Ts...> {
			uint32_t	accum = 0;

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
				++accum;
				return _advance(IConst<0>());
			}
		};
	}
}
