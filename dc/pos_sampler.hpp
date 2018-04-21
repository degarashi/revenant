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
	};
}
