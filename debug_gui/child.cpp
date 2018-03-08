#include "child.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		ChildPush::ChildPush(const char* id, const ImVec2& size, bool border, ImGuiWindowFlags flags):
			_result(ImGui::BeginChild(id, size, border, flags)),
			_destroy(false)
		{}
		ChildPush::ChildPush(ChildPush&& c):
			_result(c._result),
			_destroy(c._destroy)
		{
			c._destroy = true;
		}
		ChildPush::~ChildPush() {
			close();
		}
		void ChildPush::close() {
			if(!_destroy) {
				ImGui::EndChild();
				_destroy = true;
			}
		}
		ChildPush::operator bool () const noexcept {
			return _result;
		}
	}
}
