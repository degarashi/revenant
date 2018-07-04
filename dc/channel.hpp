#pragma once
#include "../handle/animation.hpp"
#include "channel_if.hpp"

namespace rev::dc {
	struct JChannel : IChannelT<IJointQuery> {
		HPoseFrame		_output;
		HJat			_jat;

		void apply(const IJointQuery& q, const float t) const override;
	};
}
