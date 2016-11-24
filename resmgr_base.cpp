#include "resmgr_base.hpp"

namespace rev {
	std::vector<ResMgrBase*> ResMgrBase::s_mgr;
	ResMgrBase::~ResMgrBase() {
		// ポインタのコピーは例外を発さないのでnoexcept
		const auto itr = std::find(s_mgr.begin(), s_mgr.end(), this);
		if(itr != s_mgr.end())
			s_mgr.erase(itr);
	}
	HRes ResMgrBase::LoadResource(const URI& uri) {
		for(auto* m : s_mgr) {
			if(HRes r = m->loadResource(uri))
				return r;
		}
		return HRes();
	}
}
