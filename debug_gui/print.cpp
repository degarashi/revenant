#include "print.hpp"
#include "../imgui/imgui.h"
#include "frea/vector.hpp"
#include <cstring>

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
			bool _Edit(int32_t& i) {
				return ImGui::InputInt("", &i, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
			}
			bool _Edit(std::string& s) {
				auto buff = s;
				buff.resize(s.size()+256);
				if(ImGui::InputText("", buff.data(), buff.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
					s.assign(buff.cbegin(), buff.cbegin()+std::strlen(buff.c_str()));
					return true;
				}
				return false;
			}
			template <class T, ENABLE_IF_I(std::is_integral<T>{})>
			bool _Edit(T& i) {
				auto p = MakeEditProxy<int>(i);
				return _Edit(p);
			}
			template <class T, ENABLE_IF_I(std::is_floating_point<T>{})>
			bool _Edit(T& i) {
				auto p = MakeEditProxy<float>(i);
				return _Edit(p);
			}
			template bool _Edit<int8_t>(int8_t& t);
			template bool _Edit<uint8_t>(uint8_t& t);
			template bool _Edit<int16_t>(int16_t& t);
			template bool _Edit<uint16_t>(uint16_t& t);
			template bool _Edit<uint32_t>(uint32_t& t);
			template bool _Edit<int64_t>(int64_t& t);
			template bool _Edit<uint64_t>(uint64_t& t);
			template bool _Edit<double>(double& t);

			bool _Edit(lubee::SizeI& s) {
				return _Edit(reinterpret_cast<frea::IVec2&>(s));
			}
			bool _Edit(lubee::SizeF& s) {
				return _Edit(reinterpret_cast<frea::Vec2&>(s));
			}

			bool _Slider(float& f, const float v_min, const float v_max) {
				return ImGui::SliderFloat("", &f, v_min, v_max);
			}
			bool _Slider(int& i, const int v_min, const int v_max) {
				return ImGui::SliderInt("", &i, v_min, v_max);
			}
			template <class T, ENABLE_IF_I(std::is_integral<T>{})>
			bool _Slider(T& t, const T v_min, const T v_max) {
				auto p = MakeEditProxy<int>(t);
				return _Slider(p, v_min, v_max);
			}
			template <class T, ENABLE_IF_I(std::is_floating_point<T>{})>
			bool _Slider(T& t, const T v_min, const T v_max) {
				auto p = MakeEditProxy<float>(t);
				return _Slider(p, v_min, v_max);
			}
		}
	}
}
