#pragma once
#include "spine/src/enum.hpp"
#include "frea/src/vector.hpp"

namespace rev {
	DefineEnum(
		Corner,
		(LeftTop)(RightTop)(LeftBottom)(RightBottom)
	);
	struct CornerOverlay {
		using Vec2 = frea::Vec2;
		Corner	pivot;
		Vec2	dist;
		float	alpha;

		int applyWindowPos() const;
	};
}
