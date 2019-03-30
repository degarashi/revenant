#pragma once
#include "lane_if.hpp"

namespace rev {
	struct Lane : LaneRaw {
		Lane			*pNext;
		Lane(const HTexMem2D& hT, const lubee::RectI& r);
	};
}
