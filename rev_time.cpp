#include "rev_time.hpp"

namespace rev {
	namespace {
		Timepoint	g_beginTime;
	}
	void RecordApplicationBeginTime() {
		g_beginTime = Clock::now();
	}
	Timepoint GetBeginTime() {
		return g_beginTime;
	}
	Duration GetCurrentTime() {
		return Clock::now() - g_beginTime;
	}
}
