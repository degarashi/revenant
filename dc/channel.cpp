#include "channel.hpp"
#include "sampler_if.hpp"
#include "lubee/error.hpp"
#include "jat_if.hpp"
#include "dc/node.hpp"

namespace rev::dc {
	float Channel::length() const {
		return _position->length();
	}
	void Channel::apply(const IJointQuery& q, const float t) const {
		D_Assert0(_position->numKey() == _sampler->numKey());
		const auto pos = _position->position(t);
		auto& node = _jat->findJoint(q);
		_sampler->sample(node.refPose(), pos.idx[0], pos.idx[1], pos.time);
	}
}
