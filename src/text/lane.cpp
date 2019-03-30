#include "lane.hpp"

namespace rev {
	Lane::Lane(const HTexMem2D& hT, const lubee::RectI& r):
		LaneRaw{hT, r},
		pNext(nullptr)
	{}
}
