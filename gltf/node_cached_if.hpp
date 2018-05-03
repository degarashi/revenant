#pragma once
#include "enums.hpp"
#include "../dc/node.hpp"

namespace rev::draw {
	struct Token;
	using Token_SP = std::shared_ptr<Token>;
}
namespace rev::gltf {
	using JointId = dc::JointId;

	struct NodeParam_USem : dc::NodeParam {
		virtual draw::Token_SP getSemantic(JointId id, USemantic sem) const = 0;
		virtual draw::Token_SP getViewport() const = 0;
	};
}
