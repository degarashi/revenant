#pragma once
#include "drawtoken_t.hpp"
#include "frea/vector.hpp"
#include <cstring>
#include <memory>

namespace rev {
	namespace draw {
		void Unif_Vec_Exec(std::size_t idx, GLint id, const void* ptr, std::size_t n);
		template <class T, std::size_t DN>
		class Unif_Vec : public Uniform<Unif_Vec<T,DN>> {
			protected:
				using base_t = Uniform<Unif_Vec<T,DN>>;
				using SPT = std::shared_ptr<T>;
				SPT			_data;
				std::size_t	_nAr;

			public:
				template <
					class V,
					ENABLE_IF(frea::is_vector<V>{})
				>
				Unif_Vec(const V& v):
					Unif_Vec(v.m, V::size, 1)
				{}
				// unalignedならそのままTのメモリ配列として扱う
				template <
					class V,
					ENABLE_IF((
						frea::is_vector<V>{} &&
						!V::align
					))
				>
				Unif_Vec(const V* vp, const int n):
					Unif_Vec(vp->m, V::size, n)
				{}
				// alignedの場合、メモリの詰め直しを行う
				template <
					class V,
					  ENABLE_IF((
						frea::is_vector<V>{} &&
						V::align
					))
				>
				Unif_Vec(const V* vp, const std::size_t n):
					_nAr(n)
				{
					_data.reset(new T[V::size*n]);
					auto* dst = _data.get();
					for(std::size_t i=0 ; i<n ; i++) {
						std::memcpy(dst, vp, sizeof(T)*V::size);
						dst += V::size;
						++vp;
					}
				}
				Unif_Vec(const T* v, const std::size_t dim, const std::size_t n):
					_data(new T[dim * n]),
					_nAr(n)
				{
					std::memcpy(_data.get(), v, sizeof(T)*dim*n);
				}
				void exec() override {
					std::size_t idx = 0;
					if constexpr (std::is_integral<T>{}) {
						if constexpr (std::is_unsigned<T>{}) {
							idx = 8;
						} else
							idx = 4;
					}
					idx += DN-1;
					Unif_Vec_Exec(idx, base_t::idUnif, _data.get(), _nAr);
				}
		};
	}
}
