#pragma once
#include "clock.hpp"

namespace rev {
	void RecordApplicationBeginTime();
	Timepoint GetBeginTime();
	Duration GetCurrentTime();
}
