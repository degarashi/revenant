#pragma once
#include "drawtoken/token.hpp"
#include "gl_if.hpp"

namespace rev::draw {
	template <class Func, class... Args>
	class VState : public TokenT<VState<Func, Args...>> {
		private:
			constexpr static int NArgs = sizeof...(Args);
			using Args_t = std::tuple<Args...>;
			Args_t	_args;
			Func	_func;
			template <std::size_t... Idx>
			void _apply(std::index_sequence<Idx...>) const {
				(GL.*_func)(std::get<Idx>(_args)...);
			}
		public:
			VState(const Func f, const Args_t& args):
				_args(args),
				_func(f)
			{}
			void exec() override {
				_apply(std::make_index_sequence<NArgs>{});
			}
	};
}
