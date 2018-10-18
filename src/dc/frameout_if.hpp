#pragma once
#include "framearray.hpp"

namespace rev::dc {
	class TfNode;
	struct IFrameOut : FrameArray {
		virtual void output(TfNode& dst, std::size_t idx, float t) const = 0;
	};
}
