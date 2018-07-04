#pragma once
#include "spine/flyweight_item.hpp"

namespace rev::dc {
	using SName = spi::FlyweightItem<std::string>;
	using JointId = uint32_t;
	class TfNode;
	struct IJointQuery {
		virtual ~IJointQuery() {}
		virtual TfNode* queryJoint(JointId id) const = 0;
		virtual TfNode* queryJoint(const SName& name) const = 0;
	};
}
