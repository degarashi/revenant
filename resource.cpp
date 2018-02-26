#include "handle.hpp"

namespace rev {
	const char* Resource::getDebugName() const noexcept {
		return getResourceName();
	}
}
