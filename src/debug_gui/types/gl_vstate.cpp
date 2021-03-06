#include "../../gl/vstate.hpp"
#include "../../gl/format.hpp"
#include "../indent.hpp"
#include "../../imgui/imgui.h"
#include "../sstream.hpp"

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
		StringStream s;
		ImGui::TextUnformatted(func);
		if(n > 0) {
			va_list va;
			va_start(va, n);
			{
				bool bF = true;
				while(n != 0) {
					if(!bF) {
						s << ", \n";
					} else
						bF = false;

					s << va_arg(va, const char*);
					--n;
				}
			}
			va_end(va);

			const auto _ = debug::IndentPush();
			ImGui::TextWrapped("%s", s.output().c_str());
		}
	}
}
