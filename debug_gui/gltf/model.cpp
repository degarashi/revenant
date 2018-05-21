#include "../indent.hpp"
#include "../id.hpp"
#include "../../dc/model_if.hpp"
#include "../../dc/node.hpp"
#include "../../imgui/imgui.h"
#include "../../dc/mesh_if.hpp"
#include "../listview.hpp"
#include "../../gltf/dc_model.hpp"

namespace rev::gltf {
	const char* GLTFModel::getDebugName() const noexcept {
		return "GLTFModel";
	}
	bool GLTFModel::property(const bool edit) {
		bool mod = false;
		static int num = 0;
		ImGui::RadioButton("Node", &num, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Mesh", &num, 1);
		ImGui::SameLine();
		ImGui::RadioButton("SkinMesh", &num, 2);
		ImGui::Separator();
		if(num == 0) {
			const auto _ = debug::IndentPush();
			const auto __ = debug::IdPush("Node");
			mod |= getNode()->property(edit);
		} else if(num == 1) {
			debug::ListView(_mesh.cbegin(), _mesh.cend(), true);
		} else {
			debug::ListView(_skinmesh.cbegin(), _skinmesh.cend(), true);
		}
		return mod;
	}
}
