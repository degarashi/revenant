#pragma once
#include "../handle/animation.hpp"
#include "channel_if.hpp"

namespace rev::dc {
	struct Channel : IChannel {
		HSeekFrame		_seek;
		HPoseFrame		_output;
		HJat			_jat;

		float length() const override;
		void apply(const IJointQuery& q, const float t) const override;
	};
}
