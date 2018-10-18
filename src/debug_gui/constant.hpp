#pragma once
#include <string>
#include "lubee/src/meta/enable_if.hpp"
#include "lubee/src/meta/typelist.hpp"

namespace rev {
	namespace debug {
		extern const float EntrySpacing;
		template <class T>
		const char* DefaultDisplayFormat;

		using DefaultDisplayTypes = lubee::Types<
			double,
			float,
			int,
			unsigned int
		>;
		template <class T>
		std::string _ToString(const T t, std::true_type);
		template <class T>
		std::string _ToString(const T t, std::false_type) {
			return std::to_string(t);
		}
		template <class T>
		std::string ToString(const T t) {
			return _ToString(t, DefaultDisplayTypes::Has<T>{});
		}
	}
}
