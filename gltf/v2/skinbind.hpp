#pragma once
#include "frea/matrix.hpp"

namespace rev::dc {
	using JointId = uint32_t;
}
namespace rev::gltf::v2 {
	class Node;
	struct SkinBind {
		dc::JointId		jointId;
		frea::Mat4		invmat;
		bool			hasRoot;
	};
	struct SkinBindSet {
		using SkinBindV = std::vector<SkinBind>;
		SkinBindV		bind;
		dc::JointId		rootId;
	};
}
