#pragma once
#include <algorithm>

namespace rev::dc {
	struct FrameArray {
		virtual ~FrameArray() {}
		virtual std::size_t numFrame() const = 0;
	};
}
