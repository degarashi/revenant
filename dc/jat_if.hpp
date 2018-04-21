#pragma once
#include <string>

namespace rev::dc {
	using Name = std::string;
	using JointId = uint32_t;
	class TfNode;
	struct IJointQuery {
		virtual ~IJointQuery() {}
		virtual TfNode& query(JointId id) const = 0;
		virtual TfNode& query(const Name& name) const = 0;
	};
	struct IJointAt {
		virtual ~IJointAt() {}
		virtual TfNode& findJoint(const IJointQuery& q) const = 0;
	};
}
