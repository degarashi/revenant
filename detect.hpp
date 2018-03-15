#pragma once
#if __cplusplus >= 201700L
#include <type_traits>

namespace rev {
	namespace detail {
		template <class T>
		struct IsMethod_Noexcept : std::false_type {};
		template <class Ret, class C, class... Arg>
		struct IsMethod_Noexcept<Ret (C::*)(Arg...) noexcept> : std::true_type {
			using type = Ret (C::*)(Arg...);
		};
		template <class T>
		struct IsMethod_Const : std::false_type {};
		template <class Ret, class C, class... Arg>
		struct IsMethod_Const<Ret (C::*)(Arg...) const> : std::true_type {
			using type = Ret (C::*)(Arg...);
		};
		template <class T>
		struct IsMethod_ConstNoexcept : std::false_type {};
		template <class Ret, class C, class... Arg>
		struct IsMethod_ConstNoexcept<Ret (C::*)(Arg...) const noexcept> : std::true_type {
			using type = Ret (C::*)(Arg...);
		};

		template <class T>
		struct IsFunction_Noexcept : std::false_type {};
		template <class Ret, class... Args>
		struct IsFunction_Noexcept <Ret (*)(Args...) noexcept> : std::true_type {
			using type = Ret (*)(Args...);
		};
		template <class T>
		struct IsFunction_Normal : std::false_type {};
		template <class Ret, class... Args>
		struct IsFunction_Normal<Ret (*)(Args...)> : std::true_type {
			using type = Ret (*)(Args...);
		};
	}
	//! Tがクラスのメソッドである場合にtrue_type
	template <class T>
	struct IsMethod : std::integral_constant<bool,
		detail::IsMethod_ConstNoexcept<T>{} ||
		detail::IsMethod_Const<T>{} ||
		detail::IsMethod_Noexcept<T>{}
	> {};
	//! Tが関数、またはクラスのstaticメソッドである場合にtrue_type
	template <class T>
	struct IsFunction : std::integral_constant<bool,
		detail::IsFunction_Normal<T>{} ||
		detail::IsFunction_Noexcept<T>{}
	> {};
}
#endif
