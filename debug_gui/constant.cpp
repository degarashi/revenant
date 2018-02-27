#include "constant.hpp"

namespace rev {
	namespace debug {
		const float EntrySpacing = 12.f;
		template <>
		const char* DefaultDisplayFormat<double> = "%.6lf";
		template <>
		const char* DefaultDisplayFormat<float> = "%.3f";
		template <>
		const char* DefaultDisplayFormat<int> = "%d";
		template <>
		const char* DefaultDisplayFormat<unsigned int> = "%u";
		}
	}
}
