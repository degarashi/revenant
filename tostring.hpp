#pragma once
#include <string>
#include <sstream>

namespace rev {
	template <class T>
	std::string ToString(const T& t) {
		std::stringstream ss;
		ss << t;
		return ss.str();
	}
}
