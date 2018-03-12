#include "resource.hpp"
#include "lubee/error.hpp"

namespace rev {
	const char* Resource::getDebugName() const noexcept {
		return getResourceName();
	}
	bool Resource::guiEditor(bool) {
		Assert(false, "GUI is not implemented (%s)", typeid(*this).name());
		return false;
	}
}
