#include "joint_at.hpp"

namespace rev::dc {
	TfNode& Jat_Name::findJoint(const IJointQuery& q) const {
		return *q.queryJoint(name);
	}
	TfNode& Jat_Id::findJoint(const IJointQuery& q) const {
		return *q.queryJoint(id);
	}
}
