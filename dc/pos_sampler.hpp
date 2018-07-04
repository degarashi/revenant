#pragma once
#include "sampler_if.hpp"
#include <memory>
#include <vector>

namespace rev::dc {
	template <class T>
	using SVec = std::shared_ptr<std::vector<T>>;

	struct SeekFrame : ISeekFrame {
		SVec<float>	pos;

		SeekFrame() = default;
		SeekFrame(const SVec<float>& p);
		float length() const override;
		std::size_t numFrame() const override;
		PosP position(float t) const override;
		PosP position(std::size_t idx, float t) const override;
	};
	struct SeekFrame_cached : SeekFrame {
		mutable std::size_t		_prevFrame;

		using SeekFrame::SeekFrame;
		SeekFrame_cached();
		using SeekFrame::position;
		PosP position(float t) const override;
	};
}
