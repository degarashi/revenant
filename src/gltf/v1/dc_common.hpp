#pragma once
#include "../../handle/opengl.hpp"
#include "spine/src/flyweight_item.hpp"
#include "frea/src/matrix.hpp"
#include <vector>

namespace rev {
	namespace dc {
		class TfNode;
		using JointId = uint32_t;
	}
	namespace gltf::v1 {
		using SName = spi::FlyweightItem<std::string>;
		using GLSLName = spi::FlyweightItem<std::string>;
		struct RTUniform;
		using RTUniform_UP = std::unique_ptr<RTUniform>;
		using RTUParams = std::vector<std::pair<GLSLName, RTUniform_UP>>;
		using RTUParams_SP = std::shared_ptr<RTUParams>;

		struct SkinBind {
			SName		jointName;
			frea::Mat4	invmat;
		};
		struct SkinBindSet {
			using SkinBindV = std::vector<SkinBind>;
			SkinBindV	bind;
			frea::Mat4	bs_m;
		};
		using SkinBindSet_SP = std::shared_ptr<SkinBindSet>;
		class GLTFMesh;
		using HMesh_U = std::shared_ptr<GLTFMesh>;
	}
}
