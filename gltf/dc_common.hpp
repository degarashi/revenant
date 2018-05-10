#pragma once
#include "../handle/opengl.hpp"
#include "spine/flyweight_item.hpp"
#include <vector>

namespace rev {
	namespace dc {
		class TfNode;
		using JointId = uint32_t;
	}
	namespace gltf {
		using GLSLName = spi::FlyweightItem<std::string>;
		struct RTUniform;
		using RTUniform_UP = std::unique_ptr<RTUniform>;
		using RTUParams = std::vector<std::pair<GLSLName, RTUniform_UP>>;
		using RTUParams_SP = std::shared_ptr<RTUParams>;
	}
}
