#include "dc/animation.hpp"

namespace rev::dc {
	Animation::Animation():
		_time(0)
	{}

	void Animation::addChannel(const HChannel& c) {
		_channel.emplace_back(c);
	}
	float Animation::length() const {
		float t = 0;
		for(auto& c : _channel)
			t = std::max(t, c->length());
		return t;
	}
	void Animation::setTime(const float t) {
		_time = t;
	}
	float Animation::time() const noexcept {
		return _time;
	}
	void Animation::reset() {
		setTime(0);
	}

	bool Animation::update(const IJointQuery& q, const float dt) {
		_time += dt;
		for(auto& c : _channel)
			c->apply(q, _time);
		return _time >= length();
	}
}
