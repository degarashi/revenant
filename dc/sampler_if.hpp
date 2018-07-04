#pragma once
#include <algorithm>

namespace beat::g3 {
	class Pose;
}
namespace rev::dc {
	using Pose3 = beat::g3::Pose;
	struct FrameArray {
		virtual ~FrameArray() {}
		virtual std::size_t numFrame() const = 0;
	};
	struct IPoseSampler : FrameArray {
		virtual void sample(Pose3& dst, std::size_t idx, float t) const = 0;
	};
	struct ISeekFrame : FrameArray {
		struct PosP {
			std::size_t	idx;
			float		time;
		};
		virtual float length() const = 0;
		virtual PosP position(float t) const = 0;
		virtual PosP position(std::size_t idx, float t) const = 0;
	};
}
