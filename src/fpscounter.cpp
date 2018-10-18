#include "fpscounter.hpp"

namespace rev {
	// --------------------- FPSCounter ---------------------
	FPSCounter::FPSCounter() noexcept {
		reset();
	}
	void FPSCounter::reset() noexcept {
		_counter = _fps = 0;
		_remain = 0;
	}
	bool FPSCounter::update(const bool drawn, const Duration delta) noexcept {
		const uint64_t d_ms = std::chrono::duration_cast<Microseconds>(delta).count();
		constexpr uint64_t sec_ms = std::chrono::duration_cast<Microseconds>(Seconds(1)).count();
		const bool exceed = _remain <= d_ms;
		if(exceed) {
			_remain = sec_ms - ((d_ms - _remain) % sec_ms);
			_fps = _counter;
			_counter = 0;
		} else
			_remain -= d_ms;
		_counter += static_cast<int>(drawn);
		return exceed;
	}
	int FPSCounter::getFPS() const noexcept {
		return _fps;
	}
}
