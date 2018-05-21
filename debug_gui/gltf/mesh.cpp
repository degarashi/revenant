#include "../../gltf/dc_mesh.hpp"
#include "../sstream.hpp"
#include "../../imgui/imgui.h"
#include "../../gltf/rt_uniform.hpp"
#include "../print.hpp"
#include "../header.hpp"
#include "../entry_field.hpp"
#include "../../primitive.hpp"
#include "../../tech_if.hpp"

namespace rev::gltf {
	std::string GLTFMesh::summary_str() const {
		StringStream s;
		s << "\"" << _userName << "\"(";
		if(_bind)
			s << "SkinMesh" << ')';
		else {
			s << "NormalMesh" << ')';
		}
		return s.output();
	}
	bool GLTFMesh::property(const bool edit) {
		bool mod = false;
		if(const auto _ = debug::Header("Skin-Bindings", !_bind, true, true)) {
		}
		if(const auto _ = debug::Header("RT-Params", !_rtParams, true, true)) {
			std::size_t idx = 0;
			for(auto& rt : *_rtParams) {
				const auto _ = debug::IdPush(idx++);
				debug::Show(nullptr, rt.first);
				ImGui::Separator();
				mod |= rt.second->property(edit);
				ImGui::Spacing();
				ImGui::Spacing();
			}
		}
		ImGui::Separator();
		{
			auto f = debug::EntryField(nullptr, edit, 2);
			ImGui::Columns(1);
			if(const auto _ = debug::Header("Primitive", !_primitive, true)) {
				mod |= _primitive->property(edit);
			}
			if(const auto _ = debug::Header("Technique", !_tech, true)) {
				mod |= _tech->property(edit);
			}
			mod |= f.modified();
		}
		return mod;
	}
}
