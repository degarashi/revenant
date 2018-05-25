#pragma once
#include "enums.hpp"
#include "../dc/node.hpp"
#include "dc_common.hpp"

namespace rev::gltf {
	using JointId = dc::JointId;
	struct ISemanticSet;

	using Mat4V = std::vector<frea::Mat4>;
	struct IQueryMatrix_USem : dc::IQueryMatrix {
		virtual void exportSemantic(ISemanticSet& s, JointId id, const SkinBindSet_SP& bind, USemantic sem) const = 0;
	};
}
