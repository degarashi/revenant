#pragma once
#include "drawtoken_t.hpp"
#include "frea/vector.hpp"
#include <cstring>
#include <memory>

namespace rev {
	namespace draw {
		void Unif_Vec_Exec(int idx, GLint id, const void* ptr, int n);
		//! ivec[1-4], fvec[1-4]対応
		template <class T, int DN>
		class Unif_Vec : public Uniform<Unif_Vec<T,DN>> {
			protected:
				using base_t = Uniform<Unif_Vec<T,DN>>;
				using SPT = std::shared_ptr<T>;
				SPT		_data;
				int		_nAr;

			public:
				template <class V, ENABLE_IF(frea::is_vector<V>{})>
				Unif_Vec(const GLint id, const V& v):
					Unif_Vec(id, v.m, V::size, 1)
				{}
				template <class V, ENABLE_IF(frea::is_vector<V>{})>
				Unif_Vec(const GLint id, const V* vp, const int n):
					Unif_Vec(id, vp->m, V::size, n)
				{}
				Unif_Vec(const GLint id, const T* v, const int nElem, const int n):
					base_t(id),
					_data(new T[nElem*n]),
					_nAr(n)
				{
					std::memcpy(_data.get(), v, sizeof(T)*nElem*n);
				}
				void exec() override {
					Unif_Vec_Exec(std::is_integral<T>::value * 4 + DN-1, base_t::idUnif, _data.get(), _nAr);
				}
		};
	}
}
