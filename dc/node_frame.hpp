#include "frameout_if.hpp"
#include <vector>
#include <memory>

namespace rev::dc {
	struct Node_FrameOut : IFrameOut {
		using FVec_S = std::shared_ptr<std::vector<float>>;
		FVec_S		value;

		constexpr static std::size_t N_Tmp = 32;
		mutable float	tmp[N_Tmp];

		virtual std::size_t getNUnit() const noexcept = 0;
		std::size_t numFrame() const override;
		const float* _calcValue(std::size_t idx, float t) const;
	};
	struct Node_T_FrameOut : Node_FrameOut {
		std::size_t getNUnit() const noexcept override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
	struct Node_R_FrameOut : Node_FrameOut {
		std::size_t getNUnit() const noexcept override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
	struct Node_S_FrameOut : Node_FrameOut {
		std::size_t getNUnit() const noexcept override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
}
