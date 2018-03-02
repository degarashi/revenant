#include "print.hpp"
#include "../imgui/imgui.h"
#include "frea/vector.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			void _Show(const bool b) {
				ImGui::TextUnformatted(b ? "True" : "False");
			}
			void _Show(const char* s) {
				ImGui::TextUnformatted(s);
			}
			void _Show(const std::string& s) {
				_Show(s.c_str());
			}
			template <class T, ENABLE_IF_I(DefaultDisplayTypes::Has<T>{} || std::is_integral<T>{} || std::is_floating_point<T>{})>
			void _Show(const T& t) {
				ImGui::TextUnformatted(ToString(t).c_str());
			}
			template void _Show<double>(const double& t);
			template void _Show<float>(const float& t);
			template void _Show<int8_t>(const int8_t& t);
			template void _Show<uint8_t>(const uint8_t& t);
			template void _Show<int16_t>(const int16_t& t);
			template void _Show<uint16_t>(const uint16_t& t);
			template void _Show<int32_t>(const int32_t& t);
			template void _Show<uint32_t>(const uint32_t& t);
			template void _Show<int64_t>(const int64_t& t);
			template void _Show<uint64_t>(const uint64_t& t);
			void _Show(const lubee::SizeI& s) {
				_Show(reinterpret_cast<const frea::IVec2&>(s));
			}
			void _Show(const lubee::SizeF& s) {
				_Show(reinterpret_cast<const frea::Vec2&>(s));
			}

			bool _Edit(bool& b) {
				return ImGui::Checkbox("", &b);
			}
			bool _Edit(float& f) {
				return ImGui::InputFloat("", &f, 0.f, 0.f, -1, ImGuiInputTextFlags_EnterReturnsTrue);
			}
			bool _Edit(double& d) {
				float f = d;
				if(ImGui::InputFloat("", &f, 0.f, 0.f, -1, ImGuiInputTextFlags_EnterReturnsTrue)) {
					d = f;
					return true;
				}
				return false;
			}
			bool _Edit(int& i) {
				return ImGui::InputInt("", &i, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
			}
			bool _Edit(unsigned int& u) {
				int i = u;
				if(ImGui::InputInt("", &i, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
					u = i;
					return true;
				}
				return false;
			}
			bool _Edit(lubee::SizeI& s) {
				return _Edit(reinterpret_cast<frea::IVec2&>(s));
			}
			bool _Edit(lubee::SizeF& s) {
				return _Edit(reinterpret_cast<frea::Vec2&>(s));
			}
		}
	}
}
