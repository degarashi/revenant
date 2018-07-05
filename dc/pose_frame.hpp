#include "frameout_if.hpp"
#include <vector>
#include <memory>

namespace rev::dc {
	using FVec = std::shared_ptr<std::vector<float>>;

	struct Pose_FrameOut : IFrameOut {
		FVec		value;

		virtual std::size_t getNUnit() const noexcept = 0;
		std::size_t numFrame() const override;
		template <class Res>
		Res _calcValue(std::size_t idx, float t) const;
	};
	struct Pose_T_Sampler : Pose_FrameOut {
		std::size_t getNUnit() const noexcept override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
	struct Pose_R_Sampler : Pose_FrameOut {
		std::size_t getNUnit() const noexcept override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
	struct Pose_S_Sampler : Pose_FrameOut {
		std::size_t getNUnit() const noexcept override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
}
