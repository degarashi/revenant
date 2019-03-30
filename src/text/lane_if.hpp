#pragma once
#include "../handle/opengl.hpp"
#include "lubee/src/rect.hpp"

namespace rev {
	struct LaneRaw {
		HTexMem2D		hTex;
		lubee::RectI	rect;	//!< 管理している領域
	};
	struct ILaneAlloc {
		virtual ~ILaneAlloc() {}
		virtual bool alloc(LaneRaw& dst, size_t w) = 0;
		virtual void addFreeLane(const HTexMem2D& hTex, const lubee::RectI& rect) = 0;
		virtual void clear() = 0;
	};
}
