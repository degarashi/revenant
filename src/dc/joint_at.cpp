#include "joint_at.hpp"

namespace rev::dc {
	TfNode& Jat_Name::findJoint(const IJointQuery& q) const {
		if(auto* p = q.queryJoint(name))
			return *p;
		using std::literals::operator""s;
		throw JointNotFound("Joint name \""s + *name + "not found"s);
	}
	TfNode& Jat_Id::findJoint(const IJointQuery& q) const {
		if(auto* p = q.queryJoint(id))
			return *p;
		using std::literals::operator""s;
		throw JointNotFound("Joint id \""s + std::to_string(id) + "not found"s);
	}
}
