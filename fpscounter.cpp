#include "fpscounter.hpp"

namespace rev {
	// --------------------- FPSCounter ---------------------
	FPSCounter::FPSCounter() noexcept {
		reset();
	}
	void FPSCounter::reset() noexcept {
		_counter = _fps = 0;
		_tmBegin = Clock::now();
	}
	bool FPSCounter::update() noexcept {
		Timepoint tp = Clock::now();
		const Duration dur = tp - _tmBegin;
		if(dur >= Seconds(1)) {
			_tmBegin = tp;
			_fps = _counter;
			_counter = 0;
			return true;
		}
		++_counter;
		return false;
	}
	int FPSCounter::getFPS() const noexcept {
		return _fps;
	}
}
