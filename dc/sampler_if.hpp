#pragma once
#include <algorithm>

namespace beat::g3 {
	class Pose;
}
namespace rev::dc {
	using Pose3 = beat::g3::Pose;
	struct IFrame {
		virtual ~IFrame() {}
		virtual std::size_t numKey() const = 0;
	};
	struct IPoseSampler : IFrame {
		virtual void sample(Pose3& dst, std::size_t idx, float t) const = 0;
	};
	struct IPosSampler : IFrame {
		struct PosP {
			std::size_t	idx;
			float		time;
		};
		virtual float length() const = 0;
		virtual PosP position(float t) const = 0;
		virtual PosP position(std::size_t idx, float t) const = 0;
	};
}
