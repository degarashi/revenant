#pragma once
#include "gl_state.hpp"
#include "gl_if.hpp"
#include "lubee/logical.hpp"
#include "lubee/tuplehash.hpp"
#include "drawcmd/queue_if.hpp"

namespace rev {
	void GL_VState_Gui(const char* func, int n, ...);
	std::string GL_VState_ToStr(bool b);
	std::string GL_VState_ToStr(float f);
	std::string GL_VState_ToStr(int i);
	std::string GL_VState_ToStr(unsigned u);
	template <class T, ENABLE_IF(std::is_integral<T>{} && std::is_unsigned<T>{})>
	std::string GL_VState_ToStr(const T t) {
		return GL_VState_ToStr(unsigned(t));
	}
	template <class T, ENABLE_IF(std::is_integral<T>{} && std::is_signed<T>{})>
	std::string GL_VState_ToStr(const T t) {
		return GL_VState_ToStr(int(t));
	}
	template <class T, ENABLE_IF(std::is_floating_point<T>{})>
	std::string GL_VState_ToStr(const T t) {
		return GL_VState_ToStr(float(t));
	}

	//! OpenGLのステート値設定
	template <class Func, class... Args>
	class GL_VState : public GLState {
		private:
			constexpr static int NArgs = sizeof...(Args);
			using Args_t = std::tuple<Args...>;
			Args_t	_args;
			Func	_func;
			template <std::size_t... Idx>
			bool _compare(const GL_VState& s, std::index_sequence<Idx...>) const {
				return lubee::And_L((std::get<Idx>(_args) == std::get<Idx>(s._args))...);
			}
			template <std::size_t... Idx>
			void _guiViewer(std::index_sequence<Idx...>) const {
				GL_VState_Gui(
					GLWrap::GetFunctionName(_func),
					NArgs,
					GL_VState_ToStr(std::get<Idx>(_args)).c_str()...
				);
			}

			struct DCmd_Apply {
				Args_t	args;
				Func	func;

				template <std::size_t... Idx>
				void _apply(std::index_sequence<Idx...>) const {
					(GL.*func)(std::get<Idx>(args)...);
				}
				static void Command(const void* p) {
					auto& self = *static_cast<const DCmd_Apply*>(p);
					self._apply(std::make_index_sequence<NArgs>{});
				}
			};
		public:
			GL_VState(const Func f, const Args&... args):
				_args{args...},
				_func(f)
			{}
			void dcmd_apply(draw::IQueue& q) const override {
				q.add(DCmd_Apply{_args, _func});
			}
			std::size_t getHash() const noexcept override {
				// 適当実装だがとりあえず、これで。
				auto* ptr = (const intptr_t*)(&_func);
				return lubee::TupleHash()(_args) ^*ptr;
			}
			Type getType() const noexcept override {
				return Type::Value;
			}
			bool operator == (const GLState& s) const noexcept override {
				return _Compare(*this, s);
			}
			bool operator == (const GL_VState& s) const noexcept {
				return _func == s._func &&
					_compare(s, std::make_index_sequence<NArgs>{});
			}
			#ifdef DEBUGGUI_ENABLED
				bool property(bool) override {
					_guiViewer(std::make_index_sequence<NArgs>{});
					return false;
				}
				const char* getDebugName() const noexcept override {
					return "GL_ValueState";
				}
			#endif
	};
	template <class... Args, class... Args2>
	auto MakeGL_VState(void (IGL::*func)(Args...), const Args2... args) {
		using ret_t = GL_VState<decltype(func), Args...>;
		return std::make_shared<ret_t>(func, static_cast<Args>(args)...);
	}
}
