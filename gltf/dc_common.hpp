#pragma once
#include "../handle/opengl.hpp"
#include <vector>

namespace rev {
	namespace draw {
		struct Token;
		using Token_SP = std::shared_ptr<Token>;
	}
	namespace dc {
		class TfNode;
		using JointId = uint32_t;
	}
	namespace gltf {
		using GLSLName = std::string;
		struct RTUniform;
		using RTUniform_UP = std::unique_ptr<RTUniform>;
		using RTUParams = std::vector<std::pair<GLSLName, RTUniform_UP>>;
		using RTUParams_SP = std::shared_ptr<RTUParams>;
	}
}
