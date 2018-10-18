#pragma once
#include <stdexcept>

namespace rev::dc {
	class TfNode;
	struct IJointQuery;
	struct IJointAt {
		struct JointNotFound : std::runtime_error {
			using std::runtime_error::runtime_error;
		};
		virtual ~IJointAt() {}
		virtual TfNode& findJoint(const IJointQuery& q) const = 0;
	};
}
