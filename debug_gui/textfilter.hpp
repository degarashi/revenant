#pragma once
#include "../imgui/imgui.h"
#include <array>
#include <memory>

namespace rev {
	namespace debug {
		template <class P>
		class StateStorage;
		class TextFilter : public ImGuiTextFilter {
			private:
				using CBuff256 = std::array<char, 256>;
				using St = StateStorage<std::shared_ptr<CBuff256>>;
				CBuff256&	_buff;
			public:
				TextFilter(const ImGuiID id);
		};
	}
}
