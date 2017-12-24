#include "clock.hpp"
#include <ostream>

namespace rev {
	std::ostream& operator << (std::ostream& os, const Duration& d) {
		return os << "(duration)" << std::chrono::duration_cast<Microseconds>(d).count() << " microsec";
	}
	std::ostream& operator << (std::ostream& os, const Timepoint& t) {
		return os << "(timepoint)" << std::chrono::duration_cast<Milliseconds>(t.time_since_epoch()).count() << " ms";
	}
}
