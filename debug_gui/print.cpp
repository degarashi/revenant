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
			template <class T, ENABLE_IF_I(DefaultDisplayTypes::Has<T>{})>
			void _Show(const T& t) {
				ImGui::TextUnformatted(ToString(t).c_str());
			}
			template void _Show<double>(const double& t);
			template void _Show<float>(const float& t);
			template void _Show<int>(const int& t);
			template void _Show<unsigned int>(const unsigned int& t);
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
