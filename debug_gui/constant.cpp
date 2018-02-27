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
		template <class T>
		std::string _ToString(const T t, std::true_type) {
			char buff[64];
			std::snprintf(buff, 64, DefaultDisplayFormat<T>, t);
			return buff;
		}
		template std::string _ToString<double>(const double t, std::true_type);
		template std::string _ToString<float>(const float t, std::true_type);
		template std::string _ToString<int>(const int t, std::true_type);
		template std::string _ToString<unsigned int>(const unsigned int t, std::true_type);
	}
}
