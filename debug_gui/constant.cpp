#include "constant.hpp"

namespace rev {
	namespace debug {
		const float EntrySpacing = 12.f;
		const std::string& DefaultDisplayFormat(double) {
			static std::string ret{"%.6lf"};
			return ret;
		}
		const std::string& DefaultDisplayFormat(float) {
			static std::string ret{"%.3f"};
			return ret;
		}
		const std::string& DefaultDisplayFormat(int) {
			static std::string ret{"%d"};
			return ret;
		}
	}
}
