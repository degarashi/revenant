#pragma once
#include "../pose2d.hpp"

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, Pose2D& p) {
		p._rflag.serialize(ar);
	}
}
