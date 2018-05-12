#pragma once
#include "sampler_if.hpp"
#include <memory>
#include <vector>

namespace rev::dc {
	template <class T>
	using SVec = std::shared_ptr<std::vector<T>>;

	struct PosSampler : IPosSampler {
		SVec<float>	pos;

		PosSampler() = default;
		PosSampler(const SVec<float>& p);
		float length() const override;
		std::size_t numKey() const override;
		PosP position(float t) const override;
		PosP position(std::size_t idx, float t) const override;
	};
	struct PosSampler_cached : PosSampler {
		mutable std::size_t		_prevFrame;

		using PosSampler::PosSampler;
		PosSampler_cached();
		using PosSampler::position;
		PosP position(float t) const override;
	};
}
