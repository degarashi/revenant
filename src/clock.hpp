#pragma once
#include "spine/src/prof_clock.hpp"
#include <boost/preprocessor.hpp>

namespace rev {
	#define DEF(name)	using name = spi::prof::name;
	DEF(Clock)
	DEF(Timepoint)
	DEF(Duration)
	DEF(Hours)
	DEF(Minutes)
	DEF(Seconds)
	DEF(Milliseconds)
	DEF(Microseconds)
	DEF(Nanoseconds)
	#undef DEF
}
