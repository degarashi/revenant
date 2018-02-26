#pragma once
#include "vector.hpp"

namespace rev {
	namespace debug {
		template <class M>
		class MatrixC {
			private:
				const M& _m;
			public:
				MatrixC(const M& m):
					_m(m)
				{}
				void show(const char* display_format = DefaultDisplayFormat(typename M::value_t()).c_str()) const {
					for(int i=0 ; i<M::dim_m ; i++) {
						const IdPush id(i);
						VectorC<typename M::vec_t>(_m.m[i]).show(display_format);
					}
				}
		};
		template <class M,
			ENABLE_IF(
				frea::is_matrix<M>{} &&
				(std::is_same<typename M::value_t, float>{})
			)
		>
		class FMatrix : public MatrixC<M> {
			private:
				M&	_m;
			public:
				FMatrix(M& m):
					MatrixC<M>(m),
					_m(m)
				{}
				bool edit(const int step=1, const int step_fast=100) {
					bool ret = false;
					for(int i=0 ; i<M::dim_m ; i++) {
						const IdPush id(i);
						ret |= FVector<typename M::vec_t>(_m.m[i]).edit("", step, step_fast);
					}
					return ret;
				}
		};
	}
}
