#pragma once
#include "enums.hpp"
#include "../dc/node.hpp"

namespace rev::gltf {
	using JointId = dc::JointId;
	struct ISemanticSet;

	struct IQueryMatrix_USem : dc::IQueryMatrix {
		virtual void exportSemantic(ISemanticSet& s, JointId id, USemantic sem) const = 0;
		virtual void exportViewport(ISemanticSet& s) const = 0;
	};
}
