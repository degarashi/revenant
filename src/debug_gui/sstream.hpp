#pragma once
#include <sstream>

namespace rev {
	class StringStream : public std::stringstream {
		public:
			using std::stringstream::stringstream;
			std::string output();
	};
}
