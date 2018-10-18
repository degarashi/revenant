#include "dc/animation.hpp"
#include <cmath>

namespace rev::dc {
	Animation::Animation():
		_time(0)
	{}

	void Animation::addChannel(const HChannel& c) {
		_channel.emplace_back(c);
		_length = spi::none;
	}
	void Animation::append(const Animation& a) {
		_channel.insert(_channel.end(), a._channel.begin(), a._channel.end());
		_length = spi::none;
	}
	void Animation::clear() {
		_channel.clear();
		setTime(0);
	}
	float Animation::length() const {
		if(!_length) {
			float t = 0;
			for(auto& c : _channel)
				t = std::max(t, c->length());
			_length = t;
		}
		return *_length;
	}
	void Animation::setTime(const float t) {
		_time = t;
	}
	float Animation::time() const noexcept {
		return _time;
	}
	bool Animation::empty() const noexcept {
		return _channel.empty();
	}
	void Animation::loop() {
		_time = std::fmod(_time, length());
	}
	void Animation::update(const IJointQuery& q) {
		for(auto& c : _channel)
			c->apply(q, _time);
	}
	bool Animation::advance(const float dt) {
		_time += dt;
		return _time >= length();
	}
}
