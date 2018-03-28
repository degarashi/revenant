#pragma once
#include "matrix.hpp"
#include "texture.hpp"

namespace rev {
	namespace draw {
		using Token_SP = std::shared_ptr<Token>;
		template <class To, class From>
		Token_SP _MakeUniform_To(const From* t, const std::size_t nvalue);

		inline Token_SP MakeUniform(const bool* b, const std::size_t nvalue) {
			return std::make_shared<Unif_Vec<bool,1>>(b, nvalue);
		}
		inline Token_SP MakeUniform(const float* f, const std::size_t nvalue) {
			return std::make_shared<Unif_Vec<float,1>>(f, nvalue);
		}
		inline Token_SP MakeUniform(const int32_t* t, const std::size_t nvalue) {
			return std::make_shared<Unif_Vec<int32_t,1>>(t, nvalue);
		}
		inline Token_SP MakeUniform(const uint32_t* t, const std::size_t nvalue) {
			return std::make_shared<Unif_Vec<uint32_t,1>>(t, nvalue);
		}
		inline Token_SP MakeUniform(const HTex* t, const std::size_t nvalue) {
			return std::make_shared<TextureA>(t, nvalue);
		}

		// floating-point型はfloatへ変換
		template <class T, ENABLE_IF(std::is_floating_point<T>{})>
		Token_SP MakeUniform(const T* t, const std::size_t nvalue) {
			return _MakeUniform_To<float>(t, nvalue);
		}
		// signed integralはint32_tに変換
		template <class T, ENABLE_IF(std::is_integral<T>{} && std::is_signed<T>{})>
		Token_SP MakeUniform(const T* t, const std::size_t nvalue) {
			return _MakeUniform_To<int32_t>(t, nvalue);
		}
		// unsigned integralはuint32_tに変換
		template <class T, ENABLE_IF(std::is_integral<T>{} && std::is_unsigned<T>{})>
		Token_SP MakeUniform(const T* t, const std::size_t nvalue) {
			return _MakeUniform_To<uint32_t>(t, nvalue);
		}

		//! ベクトル型変数
		template <class V, ENABLE_IF(frea::is_vector<V>{})>
		Token_SP MakeUniform(const V* v, const std::size_t nvalue) {
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
		Token_SP MakeUniform(const M* m, const std::size_t nvalue) {
			// 正方形に変換
			constexpr int DIM = lubee::Arithmetic<M::dim_m, M::dim_n>::great;
			std::vector<frea::Mat_t<typename M::value_t,DIM,DIM,false>> tm(nvalue);
			for(std::size_t i=0 ; i<nvalue ; i++)
				tm[i] = m[i].template convert<DIM,DIM>();
			return MakeUniform(tm.data(), nvalue);
		}
		//! 行列Uniform変数(正方形)
		template <
			class M,
			ENABLE_IF(
				frea::is_matrix<M>{} &&
				(M::dim_m == M::dim_n)
			)
		>
		Token_SP MakeUniform(const M* m, const std::size_t nvalue) {
			return std::make_shared<Unif_Mat<typename M::value_t, M::dim_m>>(m, nvalue, true);
		}

		//! std::vectorはポインタとサイズに分割
		template <class T>
		Token_SP MakeUniform(const std::vector<T>& ar, std::size_t s=0) {
			if(s == 0)
				s = ar.size();
			return MakeUniform(ar.data(), s);
		}
		inline Token_SP MakeUniform(const std::vector<bool>& ar, std::size_t s=0) {
			if(s == 0)
				s = ar.size();
			// boolで特殊化されてる物を解除
			const std::vector<uint8_t> tmp(ar.cbegin(), ar.cbegin()+s);
			return MakeUniform(tmp);
		}
		//! std::arrayはポインタとサイズに分割
		template <class T, std::size_t N>
		Token_SP MakeUniform(const std::array<T,N>& ar, std::size_t s=0) {
			if(s == 0)
				s = ar.size();
			return MakeUniform(ar.data(), s);
		}
		//! 単体の値 -> ポインタ + サイズ(=1)
		template <class T>
		Token_SP MakeUniform(const T& t) {
			return MakeUniform(&t, 1);
		}
		//! 値を変換してから再度MakeUniform
		template <class To, class From>
		Token_SP _MakeUniform_To(const From* t, const std::size_t nvalue) {
			std::vector<To> tmp(nvalue);
			for(std::size_t i=0 ; i<nvalue ; i++)
				tmp[i] = t[i];
			return MakeUniform(tmp.data(), nvalue);
		}
	}
}
