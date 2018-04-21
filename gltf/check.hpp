#pragma once
#include "gltf/exceptions.hpp"
#include "lubee/meta/countof.hpp"
#include "spine/optional.hpp"
#include <limits>

namespace rev {
	namespace gltf {
		template <class Ar, class Value, class Cmp=std::equal_to<>>
		auto FindEnum(const Ar& ar, const Value& v, Cmp&& cmp=std::equal_to<>{}) -> spi::Optional<decltype(ar[0])> {
			const int n = countof(ar);
			for(int i=0 ; i<n ; i++) {
				if(cmp(ar[i], v))
					return ar[i];
			}
			return spi::none;
		}
		template <class Ar, class Value, class Cmp=std::equal_to<>>
		auto CheckEnum(const Ar& ar, const Value& v, Cmp&& cmp=std::equal_to<>{}) -> decltype(ar[0]) {
			if(const auto ret = FindEnum(ar, v, cmp))
				return *ret;
			throw InvalidEnum("");
		}
		template <class T>
		void CheckRange(
			const T& val,
			const T& min=std::numeric_limits<T>::min(),
			const T& max=std::numeric_limits<T>::max()
		) {
			if(val >= min &&
				val <= max)
				return;
			throw OutOfRange("");
		}
	}
}
