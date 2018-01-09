#pragma once
#include "matrix.hpp"
#include "texture.hpp"

namespace rev {
	namespace draw {
		using Token_SP = std::shared_ptr<Token>;

		inline Token_SP MakeUniform(const bool* b, const int nvalue) {
			return std::make_shared<Unif_Vec<bool,1>>(b, 1, nvalue);
		}
		template <class T, ENABLE_IF(std::is_floating_point<T>{})>
		Token_SP MakeUniform(const T* t, const int nvalue) {
			return std::make_shared<Unif_Vec<float,1>>(t, 1, nvalue);
		}
		template <class T, ENABLE_IF(std::is_integral<T>{})>
		Token_SP MakeUniform(const T* t, const int nvalue) {
			return std::make_shared<Unif_Vec<int32_t,1>>(t, 1, nvalue);
		}
		template <class V, ENABLE_IF(frea::is_vector<V>{})>
		Token_SP MakeUniform(const V* v, const int nvalue) {
			return std::make_shared<Unif_Vec<typename V::value_t, V::size>>(v, nvalue);
		}
		//! 行列Uniform変数(非正方形)
		template <
			class M,
			ENABLE_IF((
				frea::is_matrix<M>{} &&
				(M::dim_m != M::dim_n)
			))
		>
		Token_SP MakeUniform(const M* m, const int nvalue) {
			constexpr int DIM = lubee::Arithmetic<M::dim_m, M::dim_n>::great;
			std::vector<frea::Mat_t<typename M::value_t,DIM,DIM,false>> tm(nvalue);
			for(int i=0 ; i<nvalue ; i++)
				tm[i] = m[i].template convert<DIM,DIM>();
			return MakeUniform(tm.data(), nvalue);
		}
		//! 行列Uniform変数(正方形)
		template <
			class M,
			ENABLE_IF((
				frea::is_matrix<M>{} &&
				(M::dim_m == M::dim_n)
			))
		>
		Token_SP MakeUniform(const M* m, const int nvalue) {
			return std::make_shared<Unif_Mat<typename M::value_t, M::dim_m>>(m, nvalue, true);
		}
		inline Token_SP MakeUniform(const HTex* t, const int nvalue) {
			return std::make_shared<TextureA>(t, nvalue);
		}
		template <class T>
		Token_SP MakeUniform(const std::vector<T>& ar) {
			return MakeUniform(ar.data(), ar.size());
		}
		template <class T, std::size_t N>
		Token_SP MakeUniform(const std::array<T,N>& ar) {
			return MakeUniform(ar.data(), ar.size());
		}
		template <class T>
		Token_SP MakeUniform(const T& t) {
			return MakeUniform(&t, 1);
		}
	}
}
