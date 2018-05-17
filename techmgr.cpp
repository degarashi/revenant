#include "techmgr.hpp"

namespace rev {
	HTP TechMgr::loadTechPass(const std::string& name) {
		return std::make_shared<TechPass>(name);
	}
}
