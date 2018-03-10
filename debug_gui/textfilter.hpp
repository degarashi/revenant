#pragma once
#include "../imgui/imgui.h"
#include <array>

namespace rev {
	namespace debug {
		class TextFilter : public ImGuiTextFilter {
			private:
				using CBuff256 = std::array<char, 256>;
				CBuff256&	_buff;
			public:
				TextFilter(const ImGuiID id);
		};
	}
}
