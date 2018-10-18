#pragma once
#include "frea/src/matrix.hpp"
#include "../../gl_header.hpp"

namespace rev::draw {
	void Unif_Mat_Exec(std::size_t idx, GLint id, const void* ptr, std::size_t n, bool bT);

	template <class T, std::size_t MaxN>
	struct Matrix {
		GLint			unifId;
		T				data[MaxN];
		std::size_t		num,
						dim;
		bool			transpose;

		static void Command(const void* p) {
			auto& self = *static_cast<const Matrix*>(p);
			Unif_Mat_Exec(self.dim-2, self.unifId, self.data, self.num, self.transpose);
		}
	};

	// -------------------- Matrix単体 --------------------
	// 正方行列
	template <
		class M,
		ENABLE_IF(
			M::dim_m == M::dim_n
		)
	>
	auto MakeMatrix(const M& m, const GLint id, const bool transpose) {
		constexpr auto S = M::dim_m * M::dim_m;
		Matrix<typename M::value_t, S*1> ret;
		ret.unifId = id;
		ret.num = 1;
		ret.dim = M::dim_m;
		ret.transpose = transpose;

		auto* dst = ret.data;
		for(std::size_t i=0 ; i<M::dim_m ; i++)
			for(std::size_t j=0 ; j<M::dim_m ; j++)
				*dst++ = m.m[i][j];
		D_Assert0(dst == ret.data+S);
		return ret;
	}
	// 正方行列ではない物については変換
	template <
		class M,
		std::size_t MDim = lubee::Arithmetic<M::dim_m, M::dim_n>::great,
		ENABLE_IF(
			M::dim_m != M::dim_n
		)
	>
	auto MakeMatrix(const M& m, const GLint id, const bool transpose) {
		return MakeMatrix(m.template convertI<MDim,MDim>(1), id, transpose);
	}

	// -------------------- Matrix配列 --------------------
	// 正方行列
	template <
		std::size_t MaxN,
		class Itr,
		class M = std::decay_t<decltype(*std::declval<Itr>())>,
		ENABLE_IF(
			M::dim_m == M::dim_n
		)
	>
	auto MakeMatrixArray(Itr itr, const Itr itrE, const GLint id, const bool transpose) {
		const auto num = itrE-itr;
		constexpr std::size_t S = M::dim_m * M::dim_m;
		D_Assert0(std::size_t(num) <= MaxN);
		Matrix<typename M::value_t, MaxN*S> ret;
		ret.unifId = id;
		ret.num = num;
		ret.dim = M::dim_m;
		ret.transpose = transpose;

		auto* dst = ret.data;
		while(itr != itrE) {
			auto& m = *itr;
			for(std::size_t i=0 ; i<M::dim_m ; i++)
				for(std::size_t j=0 ; j<M::dim_m ; j++)
					*dst++ = m.m[i][j];
			++itr;
		}
		D_Assert0(dst == ret.data+S*num);
		return ret;
	}
	// 正方行列ではない物については変換
	template <
		std::size_t MaxN,
		class Itr,
		class M = std::decay_t<decltype(*std::declval<Itr>())>,
		std::size_t MDim = lubee::Arithmetic<M::dim_m, M::dim_n>::great,
		ENABLE_IF(
			M::dim_m != M::dim_n
		)
	>
	auto MakeMatrixArray(Itr itr, const Itr itrE, const GLint id, const bool transpose) {
		const auto num = itrE-itr;
		using value_t = typename M::value_t;
		frea::Mat_t<value_t, MDim, MDim, false> tmp[MaxN];
		auto* dst = tmp;
		while(itr != itrE) {
			*dst++ = itr->template convertI<MDim,MDim>(1);
			++itr;
		}
		D_Assert0(dst == tmp + num);
		return MakeMatrixArray<MaxN>(tmp.begin(), tmp.end(), id, transpose);
	}
}
