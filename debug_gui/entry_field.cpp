#include "entry_field.hpp"
#include "../imgui/imgui.h"
#include "spacing.hpp"

namespace rev {
	namespace debug {
		EntryField::EntryField(IdPush&& idp, const bool edit, const int columns):
			IdPush(std::move(idp)),
			Entry(0, edit, columns)
		{
			ImGui::SetColumnWidth(0, 120.f);
		}
	}
}
