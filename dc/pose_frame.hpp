#include "frameout_if.hpp"
#include "frea/quaternion.hpp"

namespace rev::dc {
	using FVec = std::shared_ptr<std::vector<float>>;

	struct Pose_T_Sampler : IFrameOut {
		constexpr static std::size_t NUnit = 3;
		FVec		value;

		std::size_t numFrame() const override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
	struct Pose_R_Sampler : IFrameOut {
		constexpr static std::size_t NUnit = 4;
		FVec		value;

		std::size_t numFrame() const override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
	struct Pose_S_Sampler : IFrameOut {
		constexpr static std::size_t NUnit = 3;
		FVec		value;

		std::size_t numFrame() const override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
}
