#include "resource.hpp"
#include "lubee/error.hpp"

namespace rev {
	const char* Resource::getDebugName() const noexcept {
		return getResourceName();
	}
	bool Resource::guiEditor(const bool redirect) {
		Assert(!redirect, "GUI is not implemented (%s)", typeid(*this).name());
		guiViewer(true);
		return false;
	}
	void Resource::guiViewer(const bool redirect) {
		Assert(!redirect, "GUI is not implemented (%s)", typeid(*this).name());
		guiEditor(true);
	}
}
