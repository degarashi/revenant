#pragma once
#include "../handle/animation.hpp"

namespace rev::dc {
	struct IJointQuery;
	struct Channel {
		HSeekFrame		_seek;
		HFrameOut		_output;
		HJat			_jat;

		float length() const;
		void apply(const IJointQuery& q, float t) const;
	};
}
