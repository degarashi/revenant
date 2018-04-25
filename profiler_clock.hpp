#pragma once
#include "clock.hpp"

namespace rev::prof {
	struct IClock {
		virtual ~IClock() {}
		virtual Timepoint now() const = 0;
	};
	struct StdClock : IClock {
		Timepoint now() const override;
	};
}
