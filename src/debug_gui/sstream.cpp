#include "sstream.hpp"

namespace rev {
	std::string StringStream::output() {
		auto ret = str();
		str(std::string());
		clear();
		return ret;
	}
}
