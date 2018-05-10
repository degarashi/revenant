#pragma once
#include "enums.hpp"
#include "../dc/node.hpp"

namespace rev {
	class UniformEnt;
	using SName = spi::FlyweightItem<std::string>;
}
namespace rev::gltf {
	using JointId = dc::JointId;

	struct NodeParam_USem : dc::NodeParam {
		virtual void exportSemantic(UniformEnt& u, const SName& uname, JointId id, USemantic sem) const = 0;
		virtual void exportViewport(UniformEnt& u, const SName& uname) const = 0;
	};
}
