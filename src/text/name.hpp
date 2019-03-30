#pragma once
#include <string>

namespace rev {
	struct FontName : std::string {
		using std::string::string;
		FontName() = default;
		FontName(const std::string& s): std::string(s) {}
	};
}
namespace std {
	template <>
	struct hash<::rev::FontName> {
		uint32_t operator()(const ::rev::FontName& fn) const {
			// MSBを必ず1にする
			return hash<std::string>()(static_cast<const std::string&>(fn)) | 0x80000000;
		}
	};
}
