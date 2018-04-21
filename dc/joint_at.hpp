#pragma once
#include "jat_if.hpp"

namespace rev::dc {
	struct Jat_Name : IJointAt {
		Name	name;
		TfNode& findJoint(const IJointQuery& q) const override;
	};
	struct Jat_Id : IJointAt {
		JointId	id;
		TfNode& findJoint(const IJointQuery& q) const override;
	};
}
