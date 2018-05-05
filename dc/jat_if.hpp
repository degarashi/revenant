#pragma once
#include <string>
#include "spine/flyweight_item.hpp"

namespace rev::dc {
	using SName = spi::FlyweightItem<std::string>;
	using JointId = uint32_t;
	class TfNode;
	struct IJointQuery {
		virtual ~IJointQuery() {}
		virtual TfNode& query(JointId id) const = 0;
		virtual TfNode& query(const SName& name) const = 0;
	};
	struct IJointAt {
		virtual ~IJointAt() {}
		virtual TfNode& findJoint(const IJointQuery& q) const = 0;
	};
}
