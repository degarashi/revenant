#include "debuggui_if.hpp"

namespace rev {
	const char* IDebugGui::getDebugName() const noexcept {
		return "not implemented";
	}
	bool IDebugGui::property(bool) {
		return false;
	}
	void IDebugGui::summary() const {}
}
