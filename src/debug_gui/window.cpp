#include "window.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		WindowPush::WindowPush(const char* label, bool* p_open, ImGuiWindowFlags flags):
			_result(ImGui::Begin(label, p_open, flags)),
			_destroy(false)
		{}
		WindowPush::WindowPush(const char* label, bool* p_open, const float w, const float h,const float bg_alpha_override, ImGuiWindowFlags flags):
			WindowPush(label, p_open, {w,h}, bg_alpha_override, flags)
		{}
		WindowPush::WindowPush(const char* label, bool* p_open, const ImVec2& size_on_first_use, const float bg_alpha_override, ImGuiWindowFlags flags):
			_result(ImGui::Begin(label, p_open, size_on_first_use, bg_alpha_override, flags)),
			_destroy(false)
		{}
		WindowPush::WindowPush(WindowPush&& w):
			_result(w._result),
			_destroy(w._destroy)
		{
			w._destroy = true;
		}
		WindowPush::~WindowPush() {
			if(!_destroy)
				ImGui::End();
		}
		WindowPush::operator bool () const noexcept {
			return _result;
		}
	}
}
