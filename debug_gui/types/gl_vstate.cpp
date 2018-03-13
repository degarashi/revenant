#include "../../gl_vstate.hpp"
#include "../../gl_format.hpp"
#include "../indent.hpp"
#include "../../imgui/imgui.h"

namespace rev {
	std::string GL_VState_ToStr(const float f) {
		return std::to_string(f);
	}
	std::string GL_VState_ToStr(const unsigned u) {
		std::string ret = GLFormat::QueryEnumString(u);;
		ret.append("(");
		ret.append(std::to_string(u));
		ret.append(")");
		return ret;
	}
	std::string GL_VState_ToStr(const int i) {
		if(i >= 0)
			return GL_VState_ToStr(static_cast<unsigned>(i));
		return GLFormat::QueryEnumString(i);
	}
	std::string GL_VState_ToStr(const bool b) {
		return b ? "True" : "False";
	}
	void GL_VState_Gui(const char* func, int n, ...) {
		ImGui::TextUnformatted(func);
		if(n > 0) {
			ImGui::SameLine();
			ImGui::TextUnformatted("(");

			va_list va;
			va_start(va, n);
			{
				bool bF = true;
				const auto _ = debug::IndentPush();
				while(n != 0) {
					if(!bF) {
						ImGui::SameLine();
						ImGui::TextUnformatted(",");
					} else
						bF = false;

					auto* str = va_arg(va, const char*);
					ImGui::SameLine();
					ImGui::TextUnformatted(str);
					--n;
				}
			}
			va_end(va);

			ImGui::SameLine();
			ImGui::TextUnformatted(")");
		}
	}
}
