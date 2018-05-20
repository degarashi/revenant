#include "../../dc/mesh.hpp"
#include "../../imgui/imgui.h"
#include "../id.hpp"
#include "../../primitive.hpp"
#include "../../tech_if.hpp"
#include "../entry_field.hpp"
#include "../header.hpp"

namespace rev::dc {
	bool IMesh::property(const bool edit) {
		bool mod = false;
		auto f = debug::EntryField(nullptr, edit, 2);
		ImGui::Columns(1);
		if(const auto _ = debug::Header("Primitive", !_primitive, true)) {
			mod |= _primitive->property(edit);
		}
		if(const auto _ = debug::Header("Technique", !_tech, true)) {
			mod |= _tech->property(edit);
		}
		return mod | f.modified();
	}
}
