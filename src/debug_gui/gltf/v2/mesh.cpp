#include "../../../gltf/v2/dc_mesh.hpp"
#include "../../../primitive.hpp"
#include "../../../tech_if.hpp"
#include "../../../pbr_mr.hpp"
#include "../../sstream.hpp"
#include "../../entry_field.hpp"
#include "../../header.hpp"
#include "../../column.hpp"
#include "../../../gltf/v2/dc_mesh_module.hpp"

namespace rev::gltf::v2 {
	std::string GLTFMesh::summary_str() const {
		StringStream s;
		s << "\"" << _name << "\": " << _skin->typeString();
		return s.output();
	}
	bool GLTFMesh::property(const bool edit) {
		auto f = debug::EntryField(nullptr, edit, 2);
		f.show("name", _name);
		f.show("flip", _flip);
		ImGui::Columns(1);
		bool mod = false;
		if(const auto _ = debug::Header("PBR", !_pbr)) {
			mod |= _pbr->property(edit);
		}
		if(const auto _ = debug::Header("Primitive", !_primitive, false)) {
			mod |= _primitive->property(edit);
		}
		// if(const auto _ = debug::Header("Weight", _weight.empty(), false)) {
			// const auto __ = debug::ColumnPush(2);
			// std::size_t idx=0;
			// for(auto& w : _weight)
				// f.entry(std::to_string(idx++).c_str(), w);
		// }
		return f.modified() | mod;
	}
	// bool GLTFMesh::property(const bool edit) {
		// f.entry("JointId", _jointId);
		// ImGui::Columns(1);
		// bool mod = false;
		// auto tech = getTech();
		// if(const auto _ = debug::Header("Technique", !tech, true)) {
			// mod |= tech->property(edit);
		// }
		// ImGui::Separator();
		// mod |= GLTFMeshBase::property(edit);
		// return mod;
	// }
}
