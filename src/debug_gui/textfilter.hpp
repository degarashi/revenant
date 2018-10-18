#pragma once
#include "textbuffer.hpp"
#include "../imgui/imgui.h"

namespace rev::debug {
	template <class P>
	class StateStorage;
	class TextFilter :
		public TextBuffer,
		public ImGuiTextFilter
	{
		public:
			TextFilter(const ImGuiID id);
	};
}
