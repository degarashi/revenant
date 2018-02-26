#pragma once
#include <string>

namespace rev {
	namespace debug {
		extern const float EntrySpacing;
		const std::string& DefaultDisplayFormat(double);
		const std::string& DefaultDisplayFormat(float);
		const std::string& DefaultDisplayFormat(int);
	}
}
