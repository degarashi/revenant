#pragma once
#include "sampler_if.hpp"
#include <memory>
#include <vector>

namespace rev::dc {
	template <class T>
	using SVec = std::shared_ptr<std::vector<T>>;

	struct PosSampler : IPosSampler {
		SVec<float>	pos;

		float length() const override;
		std::size_t numKey() const override;
		PosP position(float t) const override;
		PosP position(std::size_t idx, float t) const override;
	};
	struct PosSampler_cached : PosSampler {
		mutable std::size_t		_prevFrame;

		PosSampler_cached();
		using PosSampler::position;
		PosP position(float t) const override;
	};
}
