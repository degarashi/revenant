#include "if.hpp"

namespace rev {
	const char* IDebugGui::getDebugName() const noexcept {
		return "not implemented";
	}
	bool IDebugGui::property(bool) {
		return false;
	}
	void IDebugGui::summary() const {}
	std::string IDebugGui::summary_str() const {
		return getDebugName();
	}
	bool IDebugGui::summary(const FilterCB& filter) const {
		if(filter(summary_str().c_str())) {
			summary();
			return true;
		}
		return false;
	}
}
