#pragma once
#include "../pose3d.hpp"

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, Pose3D& p) {
		p._rflag.serialize(ar);
	}
}
