#pragma once
#include "enums.hpp"
#include "../dc/node.hpp"
#include "dc_common.hpp"

namespace rev::gltf {
	using JointId = dc::JointId;
	struct ISemanticSet;

	using Mat4V = std::vector<frea::Mat4>;
	struct IQueryMatrix_USem : dc::IQueryMatrix {
		virtual void exportSemantic(ISemanticSet& s, JointId id, USemantic sem) const = 0;
		virtual void exportViewport(ISemanticSet& s) const = 0;
		virtual const Mat4V& getJointMat(const frea::Mat4& node_m, const SkinBindSet_SP& bind) const = 0;
	};
}
