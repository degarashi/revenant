#pragma once
#if __cplusplus >= 201700L
#include "detect.hpp"
#include "lubee/src/meta/enable_if.hpp"

namespace rev {
	template <class T, ENABLE_IF(detail::IsMethod_Noexcept<T>{})>
	auto Remove_ConstNoexcept() -> typename detail::IsMethod_Noexcept<T>::type;
	template <class T, ENABLE_IF(detail::IsMethod_ConstNoexcept<T>{})>
	auto Remove_ConstNoexcept() -> typename detail::IsMethod_ConstNoexcept<T>::type;
	template <class T, ENABLE_IF(detail::IsMethod_Const<T>{})>
	auto Remove_ConstNoexcept() -> typename detail::IsMethod_Const<T>::type;
	template <class T, ENABLE_IF(detail::IsFunction_Noexcept<T>{})>
	auto Remove_ConstNoexcept() -> typename detail::IsFunction_Noexcept<T>::type;
	template <class T, ENABLE_IF(detail::IsFunction_Normal<T>{})>
	auto Remove_ConstNoexcept() -> typename detail::IsFunction_Normal<T>::type;

	template <
		class T,
		ENABLE_IF(
			!detail::IsMethod_Noexcept<T>{} &&
			!detail::IsMethod_ConstNoexcept<T>{} &&
			!detail::IsMethod_Const<T>{} &&
			!detail::IsFunction_Noexcept<T>{} &&
			!detail::IsFunction_Normal<T>{}
		)
	>
	auto Remove_ConstNoexcept() -> T;

	template <class T>
	using Remove_ConstNoexcept_t = decltype(Remove_ConstNoexcept<T>());
}
#endif
