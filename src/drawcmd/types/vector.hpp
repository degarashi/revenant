#pragma once
#include "frea/src/vector.hpp"
#include "../../gl_header.hpp"

namespace rev::draw {
	void Unif_Vec_Exec(std::size_t idx, GLint id, const void* ptr, std::size_t n);
	namespace detail {
		template <class T>
		std::size_t CalcIndex(const std::size_t dim) {
			std::size_t idx=0;
			if constexpr (std::is_integral<T>{}) {
				if constexpr (std::is_unsigned<T>{}) {
					idx = 8;
				} else
					idx = 4;
			} else
				idx = 0;

			idx += dim-1;
			return idx;
		}
	}
	template <class T, std::size_t MaxN>
	struct Vector {
		GLint		unifId;
		T			data[MaxN];
		std::size_t	num,
					dim;

		static void Command(const void* p) {
			auto& self = *static_cast<const Vector*>(p);
			const auto idx = detail::CalcIndex<T>(self.dim);
			Unif_Vec_Exec(idx, self.unifId, &self.data, self.num);
		}
	};
	template <class V>
	auto MakeVector(const V& v, const GLint id) {
		Vector<typename V::value_t, V::size*1> ret;
		ret.unifId = id,
		ret.num = 1,
		ret.dim = V::size;
		for(std::size_t i=0 ; i<V::size ; i++)
			ret.data[i] = v[i];
		return ret;
	}

	template <
		std::size_t MaxN,
		class Itr,
		class V = std::decay_t<decltype(*std::declval<Itr>())>
	>
	auto MakeVectorArray(Itr itr, const Itr itrE, const GLint id) {
		const auto num = itrE-itr;
		Vector<typename V::value_t, V::size*MaxN> ret;
		ret.unifId = id;
		ret.num = num;
		ret.dim = V::size;
		D_Assert0(ret.num <= MaxN);

		auto* dst = ret.data;
		while(itr != itrE) {
			auto& src = *itr;
			for(std::size_t i=0 ; i<V::size ; i++)
				*dst++ = src[i];
			++itr;
		}
		D_Assert0(dst == ret.data + V::size*num);
		return ret;
	}
}
