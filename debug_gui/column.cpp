#include "column.hpp"
#include "../imgui/imgui.h"
#include "lubee/error.hpp"

namespace rev {
	namespace debug {
		ColumnPush::ColumnPush(const int n):
			_n(n)
		{
			if(n > 1)
				ImGui::Columns(n);
		}
		ColumnPush::ColumnPush(ColumnPush&& c):
			_n(c._n)
		{
			c._n = -1;
		}
		ColumnPush::~ColumnPush() {
			if(_n > 1)
				ImGui::Columns(1);
		}
	}
}
