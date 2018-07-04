#include "channel.hpp"
#include "seekframe_if.hpp"
#include "dc/pose_frame.hpp"
#include "lubee/error.hpp"
#include "jat_if.hpp"
#include "dc/node.hpp"

namespace rev::dc {
	float Channel::length() const {
		return _seek->length();
	}
	void Channel::apply(const IJointQuery& q, const float t) const {
		D_Assert0(_seek->numFrame() == _output->numFrame());
		const auto pos = _seek->position(t);
		auto& node = _jat->findJoint(q);
		_output->output(node.refPose(), pos.idx, pos.time);
	}
}
