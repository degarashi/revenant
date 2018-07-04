#pragma once
#include "framearray.hpp"

namespace beat::g3 {
	class Pose;
}
namespace rev::dc {
	using Pose3 = beat::g3::Pose;
	//! フレーム番号 + dt を受け取り、Poseに出力
	struct IPoseFrame : FrameArray {
		virtual void sample(Pose3& dst, std::size_t idx, float t) const = 0;
	};
}
