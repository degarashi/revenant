#include "column.hpp"
#include "../imgui/imgui.h"
#include "lubee/error.hpp"

namespace rev {
	namespace debug {
		ColumnPush::ColumnPush(const int n) {
			ImGui::Columns(n);
		}
		ColumnPush::~ColumnPush() {
			ImGui::Columns(1);
		}
	}
}
