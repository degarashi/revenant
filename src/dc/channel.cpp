#include "channel.hpp"
#include "seekframe_if.hpp"
#include "dc/node_frame.hpp"
#include "lubee/src/error.hpp"
#include "jat_if.hpp"
#include "dc/node.hpp"

namespace rev::dc {
	float Channel::length() const {
		return _seek->length();
	}
	void Channel::apply(const IJointQuery& q, const float t) const {
		const auto pos = _seek->position(t);
		auto& node = _jat->findJoint(q);
		_output->output(node, pos.idx, pos.time);
		D_Assert0(_seek->numFrame() == _output->numFrame());
	}
}
