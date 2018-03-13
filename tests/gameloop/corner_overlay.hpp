#pragma once
#include "spine/enum.hpp"
#include "frea/vector.hpp"

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
