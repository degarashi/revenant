#pragma once
#include "drawtoken_t.hpp"
#include "frea/vector.hpp"
#include <cstring>
#include <memory>

namespace rev::draw {
	void Unif_Vec_Exec(std::size_t idx, GLint id, const void* ptr, std::size_t n);

	template <class T, std::size_t DN>
	class Unif_Vec : public Uniform<Unif_Vec<T,DN>> {
		protected:
			inline constexpr static std::size_t Dim = DN;
			using base_t = Uniform<Unif_Vec<T,Dim>>;
			using vec_t = frea::Vec_t<T, Dim, false>;
			// Vectorを値の配列で格納
			using SPT = std::shared_ptr<T[]>;
			SPT			_data;
			std::size_t	_nAr;

		public:
			// Vector単体
			template <
				class V,
				ENABLE_IF(
					frea::is_vector<V>{} &&
					V::size == Dim
				)
			>
			Unif_Vec(const V& v):
				Unif_Vec(v.m, 1)
			{}
			// unalignedならそのままTのメモリ配列として扱う
			template <
				class V,
				ENABLE_IF(
					frea::is_vector<V>{} &&
					V::size == Dim &&
					!V::align
				)
			>
			Unif_Vec(const V* vp, const std::size_t n):
				Unif_Vec(vp->m, n)
			{}
			// alignedの場合、メモリの詰め直しを行う
			template <
				class V,
				  ENABLE_IF(
					frea::is_vector<V>{} &&
					V::size == Dim &&
					V::align
				)
			>
			Unif_Vec(const V* vp, const std::size_t n):
				_nAr(n)
			{
				_data.reset(new T[Dim * n]);
				auto* dst = _data.get();
				for(std::size_t i=0 ; i<n ; i++) {
					std::memcpy(dst, vp, sizeof(T) * Dim);
					dst += Dim;
					++vp;
				}
				D_Assert0(dst == _data.get() + n);
			}
			Unif_Vec(const T* v, const std::size_t n):
				_data(new T[Dim * n]),
				_nAr(n)
			{
				std::memcpy(_data.get(), v, sizeof(T)*Dim*n);
			}
			void exec() override {
				std::size_t idx;
				if constexpr (std::is_integral<T>{}) {
					if constexpr (std::is_unsigned<T>{}) {
						idx = 8;
					} else
						idx = 4;
				} else
					idx = 0;

				idx += Dim-1;
				Unif_Vec_Exec(idx, base_t::idUnif, _data.get(), _nAr);
			}
	};
}
