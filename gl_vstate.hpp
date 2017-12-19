#pragma once
#include "gl_state.hpp"
#include "gl_if.hpp"
#include "lubee/logical.hpp"
#include "lubee/tuplehash.hpp"

namespace rev {
	//! OpenGLのステート値設定
	template <class Func, class... Args>
	class GL_VState : public GLState {
		private:
			constexpr static int NArgs = sizeof...(Args);
			using Args_t = std::tuple<Args...>;
			Args_t	_args;
			Func	_func;
			template <std::size_t... Idx>
			void _apply(std::index_sequence<Idx...>) const {
				(GL.*_func)(std::get<Idx>(_args)...);
			}
			template <std::size_t... Idx>
			bool _compare(const GL_VState& s, std::index_sequence<Idx...>) const {
				return lubee::And_L((std::get<Idx>(_args) == std::get<Idx>(s._args))...);
			}
		public:
			GL_VState(const Func f, const Args&... args):
				_args{args...},
				_func(f)
			{}
			std::size_t getHash() const noexcept override {
				// 適当実装だがとりあえず、これで。
				auto* ptr = (const intptr_t*)(&_func);
				return lubee::TupleHash()(_args) ^*ptr;
			}
			void apply() const override {
				_apply(std::make_index_sequence<NArgs>{});
			}
			bool operator == (const GLState& s) const noexcept override {
				return _Compare(*this, s);
			}
			bool operator == (const GL_VState& s) const noexcept {
				return _compare(s, std::make_index_sequence<NArgs>{});
			}
	};
	template <class... Args, class... Args2>
	auto MakeGL_VState(void (IGL::*func)(Args...), const Args2... args) {
		using ret_t = GL_VState<decltype(func), Args...>;
		return std::make_shared<ret_t>(func, static_cast<Args>(args)...);
	}
}
