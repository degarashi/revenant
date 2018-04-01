#pragma once
#include "vector.hpp"
#include "matrix.hpp"
#include "texture.hpp"
#include <type_traits>

namespace rev::draw {
	using Token_SP = std::shared_ptr<Token>;

	namespace detail {
		template <class T, ENABLE_IF(std::is_floating_point_v<T>)>
		GLfloat NumberCnv(T);
		template <class T, ENABLE_IF(std::is_integral_v<T> && std::is_unsigned_v<T>)>
		GLuint NumberCnv(T);
		template <class T, ENABLE_IF(std::is_integral_v<T> && std::is_signed_v<T>)>
		GLint NumberCnv(T);
		template <class T>
		using NumberCnv_t = decltype(NumberCnv(std::declval<T>()));

		template <class T, ENABLE_IF(frea::is_vector<T>{})>
		Unif_VecA<NumberCnv_t<typename T::value_t>, T::size> Detect(const std::vector<T>&);
		template <class T, ENABLE_IF(frea::is_vector<T>{})>
		Unif_Vec<NumberCnv_t<typename T::value_t>, T::size> Detect(const T&);
		template <
			class Itr,
			class V = std::decay_t<decltype(*std::declval<Itr>())>,
			ENABLE_IF(frea::is_vector<V>{})
		>
		Unif_VecA<NumberCnv_t<typename V::value_t>, V::size> Detect(Itr, Itr);

		template <class M, ENABLE_IF(frea::is_matrix<M>{})>
		Unif_MatA<NumberCnv_t<typename M::value_t>, M::dim_m> Detect(const std::vector<M>&);
		template <class M, ENABLE_IF(frea::is_matrix<M>{})>
		Unif_Mat<NumberCnv_t<typename M::value_t>, M::dim_m> Detect(const M&);
		template <
			class Itr,
			class M = std::decay_t<decltype(*std::declval<Itr>())>,
			ENABLE_IF(frea::is_matrix<M>{})
		>
		Unif_MatA<NumberCnv_t<typename M::value_t>, M::dim_m> Detect(Itr, Itr);

		template <class T, ENABLE_IF(std::is_arithmetic_v<T>)>
		Unif_VecA<NumberCnv_t<T>, 1> Detect(const std::vector<T>&);
		template <class T, ENABLE_IF(std::is_arithmetic_v<T>)>
		Unif_Vec<NumberCnv_t<T>, 1> Detect(const T&);
		template <
			class Itr,
			class T = std::decay_t<decltype(*std::declval<Itr>())>,
			ENABLE_IF(!frea::is_vector<T>{})
		>
		Unif_VecA<NumberCnv_t<T>, 1> Detect(Itr, Itr);

		Texture Detect(const HTexC&);
		TextureA Detect(const std::vector<HTexC>&);
		template <
			class Itr,
			class T = std::decay_t<decltype(*std::declval<Itr>())>,
			ENABLE_IF((std::is_same_v<T, HTexC>))
		>
		TextureA Detect(Itr, Itr);
	}

	// 単体またはstd::vector<>
	template <class T>
	Token_SP MakeUniform(T&& t) {
		using U = decltype(detail::Detect(std::forward<T>(t)));
		return Token_SP(new U(std::forward<T>(t)));
	}
	// 範囲指定による複数要素
	template <class Itr>
	Token_SP MakeUniform(const Itr itr, const Itr itrE) {
		using U = decltype(detail::Detect(itr, itrE));
		return Token_SP(new U(itr, itrE));
	}
}
