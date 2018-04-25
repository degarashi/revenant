#include "profiler_clock.hpp"

namespace rev::prof {
	// -------------------- StdClock --------------------
	Timepoint StdClock::now() const {
		return Clock::now();
	}
}
