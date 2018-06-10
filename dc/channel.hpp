#pragma once
#include "../handle/model.hpp"
#include "channel_if.hpp"

namespace rev::dc {
	struct Channel : IChannel {
		HPosSampler		_position;
		HPoseSampler	_sampler;
		HJat		_jat;

		float length() const override;
		void apply(const IJointQuery& q, const float t) const override;
	};
}
