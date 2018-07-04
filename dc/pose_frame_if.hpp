#pragma once
#include "framearray.hpp"

namespace rev::dc {
	template <class Dst>
	struct IFrameOut : FrameArray {
		virtual void output(Dst& dst, std::size_t idx, float t) const = 0;
	};
}
