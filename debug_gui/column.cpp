#include "column.hpp"
#include "../imgui/imgui.h"
#include "lubee/error.hpp"

namespace rev {
	namespace debug {
		ColumnPush::ColumnPush(const int n, const bool border):
			_prev(ImGui::GetColumnsCount()),
			_n(n)
		{
			if(_prev > 1)
				ImGui::Columns(1);
			if(n > 1)
				ImGui::Columns(n, nullptr, border);
		}
		ColumnPush::ColumnPush(ColumnPush&& c):
			_prev(c._prev),
			_n(c._n)
		{
			c._prev = -1;
		}
		ColumnPush::~ColumnPush() {
			if(_prev >= 1) {
				ImGui::Columns(1);
				ImGui::Columns(_prev);
			}
		}
	}
}
