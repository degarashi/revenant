#include "frameout_if.hpp"
#include <vector>
#include <memory>

namespace rev::dc {
	using FVec = std::shared_ptr<std::vector<float>>;

	struct Node_FrameOut : IFrameOut {
		FVec		value;

		virtual std::size_t getNUnit() const noexcept = 0;
		std::size_t numFrame() const override;
		template <class Res>
		Res _calcValue(std::size_t idx, float t) const;
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
